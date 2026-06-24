// @vitest-environment jsdom
/**
 * Specs for the Clay location-search component.
 *
 * The component runs inside Clay's config webview, so its logic is pure DOM:
 * restoring a saved selection, debouncing the geocoder, rendering suggestions
 * safely, and persisting the chosen coordinates. A regression here either loses
 * the user's location or, worse, injects markup from an attacker-named place.
 *
 * The webview globals are provided by jsdom. The main suite stubs
 * XMLHttpRequest so the geocoder is never actually hit and the timing stays
 * deterministic. The opt-in live block (RUN_LIVE=1) hits the real API.
 */

import { describe, test, expect, beforeEach, afterEach, vi } from 'vitest';
import component from './location-component.js';
import { fetchRequest } from '../../../testing/fetch-request.js';

/**
 * Builds a component the way Clay does.
 * 
 * A root from the template, with set/get/initialize bound to a context 
 * exposing $element and config. Returns that context and the elements 
 * the specs assert against.
 * 
 * @param {Object=} config
 * @return {Object}
 */
function mount(config) {
  const holder = document.createElement('div');
  holder.innerHTML = component.template;
  const root = holder.firstChild;

  const ctx = { $element: [root], config: config || {} };
  ctx.set = component.manipulator.set.bind(ctx);
  ctx.get = component.manipulator.get.bind(ctx);
  ctx.initialize = component.initialize.bind(ctx);

  return {
    ctx,
    root,
    query: root.querySelector('.loc-query'),
    hidden: root.querySelector('.loc-value'),
    list: root.querySelector('.loc-list')
  };
}

/**
 * Captures every XMLHttpRequest the component opens.
 * 
 * So a spec can inspect the url and hand back a canned geocoder 
 * response without touching the network.
 * 
 * @return {Array}
 */
function installFakeXhr() {
  const sent = [];
  class FakeXhr {
    open(method, url) {
      this.method = method;
      this.url = url;
    }

    send() {
      sent.push(this);
    }

    /**
     * Drives the response the component is waiting on.
     */
    respond(body, status) {
      this.status = status === undefined ? 200 : status;
      this.responseText = body;
      if (this.onload) {
        this.onload();
      }
    }

    /**
     * Drives a network failure the component is waiting on.
     */
    error() {
      if (this.onerror) {
        this.onerror();
      }
    }
  }

  global.XMLHttpRequest = FakeXhr;
  return sent;
}

/**
 * Types into the search box exactly as a user would.
 * 
 * Sets the value, then fires input.
 * 
 * @param {Element} query
 * @param {string} text
 */
function type(query, text) {
  query.value = text;
  query.dispatchEvent(new Event('input'));
}

describe('manipulator', () => {
  /** A saved selection must repopulate the box and round-trip its JSON, else the watch loses the chosen place. */
  test('restores the saved label and persists the value verbatim', () => {
    const { ctx, query, hidden } = mount();
    const saved = JSON.stringify({ lat: 33.4, lon: -112, label: 'Phoenix, Arizona, United States' });

    ctx.set(saved);

    expect(query.value).toBe('Phoenix, Arizona, United States');
    expect(hidden.value).toBe(saved);
    expect(ctx.get()).toBe(saved);
  });

  /** Corrupt saved data must not throw and must leave the box empty, never rendering garbage. */
  test('leaves the search box empty on malformed saved data', () => {
    const { ctx, query } = mount();

    expect(() => ctx.set('not json')).not.toThrow();
    expect(query.value).toBe('');
  });
});

describe('initialize', () => {
  let xhrs;

  beforeEach(() => {
    vi.useFakeTimers();
    xhrs = installFakeXhr();
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  /** A 1-char query would spam the geocoder on every keystroke and show a stale dropdown. */
  test('makes no request for queries shorter than 2 chars', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'P');
    vi.advanceTimersByTime(300);

    expect(xhrs).toHaveLength(0);
    expect(mounted.list.classList.contains('show')).toBe(false);
  });

  /** A missed debounce floods the API, and a wrong endpoint geocodes against the wrong service. */
  test('debounces 300ms then queries the Open-Meteo geocoder with the typed term', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Phoenix');
    vi.advanceTimersByTime(299);

    expect(xhrs).toHaveLength(0);

    vi.advanceTimersByTime(1);

    expect(xhrs).toHaveLength(1);
    expect(xhrs[0].url).toContain('geocoding-api.open-meteo.com');
    expect(xhrs[0].url).toContain('name=Phoenix');
    expect(xhrs[0].url).toContain('count=5');
  });

  /** A place name containing HTML must render as inert text, never inject markup into the config page. */
  test('renders suggestions as text, not markup', () => {
    const evil = '<img src=x onerror=alert(1)>';

    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'evil');
    vi.advanceTimersByTime(300);

    xhrs[0].respond(JSON.stringify({ results: [{ name: evil, latitude: 1, longitude: 2 }] }));

    const images = mounted.list.querySelectorAll('img');
    const text = mounted.list.querySelector('.loc-item').textContent;

    expect(images).toHaveLength(0);
    expect(text).toBe(evil);
  });

  /** A mis-joined label shows the wrong place or a comma-littered string when parts are missing. */
  test('formats the label as name, admin1, country and drops missing parts', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'city');
    vi.advanceTimersByTime(300);

    xhrs[0].respond(JSON.stringify({ results: [
      { name: 'Phoenix', admin1: 'Arizona', country: 'United States', latitude: 1, longitude: 2 },
      { name: 'Berlin', latitude: 3, longitude: 4 }
    ] }));

    const items = mounted.list.querySelectorAll('.loc-item');
    expect(items[0].textContent).toBe('Phoenix, Arizona, United States');
    expect(items[1].textContent).toBe('Berlin');
  });

  /** If selecting a result fails to persist its coordinates, the watch must geocode itself or loses the place. */
  test('persists the chosen coordinates and label and closes the list on selection', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Phoenix');
    vi.advanceTimersByTime(300);

    xhrs[0].respond(JSON.stringify({ results: [
      { name: 'Phoenix', admin1: 'Arizona', country: 'United States', latitude: 33.4, longitude: -112 }
    ] }));

    mounted.list.querySelector('.loc-item').dispatchEvent(new MouseEvent('click', { bubbles: true }));

    const saved = JSON.parse(mounted.hidden.value);

    expect(saved).toEqual({
      lat: 33.4,
      lon: -112,
      label: 'Phoenix, Arizona, United States'
    });
    expect(mounted.query.value).toBe('Phoenix, Arizona, United States');
    expect(mounted.list.classList.contains('show')).toBe(false);
  });

  /** Editing the query after a selection must drop the stored coordinates, so the watch never saves a label that disagrees with its lat/lon. */
  test('clears the stored coordinates when the query is edited', () => {
    const mounted = mount();
    mounted.ctx.initialize();
    mounted.hidden.value = JSON.stringify({ lat: 33.4, lon: -112, label: 'Phoenix' });

    type(mounted.query, 'Phoeni');

    expect(mounted.hidden.value).toBe('');
  });

  /** Rapid keystrokes must collapse into one request, not one per character that floods the geocoder. */
  test('coalesces rapid keystrokes into a single request', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Ph');
    vi.advanceTimersByTime(200);

    type(mounted.query, 'Pho');
    vi.advanceTimersByTime(300);

    expect(xhrs).toHaveLength(1);
    expect(xhrs[0].url).toContain('name=Pho');
  });

  /** A slower earlier request resolving last must not overwrite the latest query's suggestions, or the user persists the wrong city. */
  test('ignores a stale response that resolves after a newer query', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Pho');
    vi.advanceTimersByTime(300);

    type(mounted.query, 'Phoenix');
    vi.advanceTimersByTime(300);

    // the newer "Phoenix" request resolves first, then the stale "Pho" one lands
    xhrs[1].respond(JSON.stringify({ results: [{ name: 'Phoenix', latitude: 1, longitude: 2 }] }));
    xhrs[0].respond(JSON.stringify({ results: [{ name: 'Phonsavan', latitude: 3, longitude: 4 }] }));

    const items = mounted.list.querySelectorAll('.loc-item');

    expect(items).toHaveLength(1);
    expect(items[0].textContent).toBe('Phoenix');
  });

  /** A network failure must hide the list rather than leave a stale dropdown or throw. */
  test('hides the list when the geocoder request errors', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Phoenix');
    vi.advanceTimersByTime(300);

    const fail = () => xhrs[0].error();

    expect(fail).not.toThrow();
    expect(mounted.list.classList.contains('show')).toBe(false);
  });

  /** Clicking outside the component must dismiss the dropdown, else a stale suggestion list lingers over the rest of the formounted. */
  test('dismisses the dropdown when clicking outside the component', () => {
    const mounted = mount();
    mounted.ctx.initialize();

    type(mounted.query, 'Phoenix');
    vi.advanceTimersByTime(300);

    xhrs[0].respond(JSON.stringify({ results: [{ name: 'Phoenix', latitude: 1, longitude: 2 }] }));

    expect(mounted.list.classList.contains('show')).toBe(true);

    document.body.dispatchEvent(new MouseEvent('click', { bubbles: true }));

    expect(mounted.list.classList.contains('show')).toBe(false);
  });
});

/**
 * Live geocoding check against the real Open-Meteo geocoding API (no key).
 * 
 * Opt-in via RUN_LIVE=1 - catches the upstream dropping the fields the 
 * dropdown reads.
 */
describe.skipIf(process.env.RUN_LIVE !== '1')('live geocoding', () => {
  /**
   * Mirrors the url initialize() builds.
   * 
   * Hit directly to assert the result shape.
   */
  const geocode = (query) => new Promise((resolve) => {
    const url = 'https://geocoding-api.open-meteo.com/v1/search?name=' +
      encodeURIComponent(query) + '&count=5&language=en&format=json';
    fetchRequest(url, (err, body) => resolve(err ? null : JSON.parse(body)));
  });

  /** The dropdown renders name and stores latitude/longitude - upstream dropping them blanks the suggestions or saves no coords. */
  test('returns results carrying the fields the dropdown reads', async () => {
    const data = await geocode('Phoenix');
    expect(Array.isArray(data && data.results)).toBe(true);

    const top = data.results[0];
    expect(typeof top.name).toBe('string');
    expect(typeof top.latitude).toBe('number');
    expect(typeof top.longitude).toBe('number');
  });
});
