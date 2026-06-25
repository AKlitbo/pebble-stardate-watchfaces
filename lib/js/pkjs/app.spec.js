// @vitest-environment jsdom
/**
 * Specs for the shared PebbleKit JS bootstrap.
 *
 * This module is the only copy of the weather/settings glue both faces run, so
 * the hardening it carries (HTTP-status handling, untrusted-payload guards,
 * coordinate range checks, and the change-gated refetch) is tested here once.
 *
 * The webview globals (localStorage) come from jsdom; XMLHttpRequest is stubbed
 * so nothing touches the network. Clay and the per-face `message_keys` alias are
 * only loaded inside startPebbleApp, so the exported helpers test without them -
 * seedConfigFromWatch takes its message-key map as an argument.
 */

import { describe, test, expect, beforeEach, vi } from 'vitest';
import app from './app.js';

// stands in for the per-face generated message_keys: each key name maps to
// itself, so payloads and stored config use readable string keys
const messageKeys = new Proxy({}, { get: (_target, prop) => prop });

/**
 * Captures every XMLHttpRequest the code opens and lets a spec drive the
 * response, an error, or a timeout.
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

    respond(httpStatus, body) {
      this.status = httpStatus;
      this.responseText = body;
      this.onload();
    }

    fail() {
      this.onerror();
    }

    timeOut() {
      this.ontimeout();
    }
  }

  global.XMLHttpRequest = FakeXhr;
  return sent;
}

describe('request', () => {
  /** A successful response must reach the callback as data with no error. */
  test('reports a 2xx response as success with the body', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].respond(200, '{"ok":1}');

    expect(callback).toHaveBeenCalledWith(null, '{"ok":1}');
  });

  /** A non-2xx must be flagged as an error while still forwarding the body so a provider can read a structured error. */
  test('reports a non-2xx as an http error but still forwards the body', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].respond(404, '{"cod":404}');

    expect(callback).toHaveBeenCalledWith('http 404', '{"cod":404}');
  });

  /** A transport failure must surface as a network error, never a silent success. */
  test('reports a transport failure as a network error', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].fail();

    expect(callback).toHaveBeenCalledWith('network error');
  });

  /** A timeout must surface distinctly so the caller can show a clear status. */
  test('reports a timeout', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].timeOut();

    expect(callback).toHaveBeenCalledWith('timeout');
  });
});

describe('validCoord', () => {
  /** An in-range pair is the only thing that should be forwarded to a provider. */
  test('accepts an in-range coordinate pair', () => {
    const result = app.validCoord(33.4, -112);

    expect(result).toBe(true);
  });

  /** Out-of-range or non-numeric coordinates must be rejected before they reach the network. */
  test.each([
    ['latitude too high', 91, 0],
    ['latitude too low', -91, 0],
    ['longitude too high', 0, 181],
    ['longitude too low', 0, -181],
    ['non-numeric latitude', '33', -112],
    ['missing longitude', 33, undefined]
  ])('rejects %s', (label, lat, lon) => {
    const result = app.validCoord(lat, lon);

    expect(result).toBe(false);
  });
});

describe('getManualLocation', () => {
  /** A valid saved place must yield its coordinates and label so the watch fetches the right city. */
  test('returns coordinates and label for an in-range saved location', () => {
    const config = { LOCATION_NAME: JSON.stringify({ lat: 33.4, lon: -112, label: 'Phoenix' }) };

    const result = app.getManualLocation(config);

    expect(result).toEqual({ coords: { lat: 33.4, lon: -112 }, label: 'Phoenix' });
  });

  /** An out-of-range blob must be treated as no manual location, never forwarded verbatim. */
  test('rejects an out-of-range saved coordinate', () => {
    const config = { LOCATION_NAME: JSON.stringify({ lat: 999, lon: -112, label: 'Bad' }) };

    const result = app.getManualLocation(config);

    expect(result).toBeNull();
  });

  /** Malformed JSON must not throw and must fall back to no location. */
  test('returns null for a malformed saved value', () => {
    const config = { LOCATION_NAME: 'not json' };

    const result = app.getManualLocation(config);

    expect(result).toBeNull();
  });

  /** An unset location must be null so the caller falls back to GPS or a no-location status. */
  test('returns null when no location is saved', () => {
    const result = app.getManualLocation({});

    expect(result).toBeNull();
  });
});

describe('seedConfigFromWatch', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /**
   * Reads the value seedConfigFromWatch persisted under a given key.
   * @param {string} key
   * @return {*}
   */
  function stored(key) {
    return JSON.parse(localStorage.getItem('clay-settings'))[key];
  }

  /** A valid date format string from the watch must seed the config so it opens with the real value. */
  test('copies a valid string DATE_FORMAT into the store', () => {
    app.seedConfigFromWatch(messageKeys, { DATE_FORMAT: '%Y.%m.%d' });

    const result = stored('DATE_FORMAT');

    expect(result).toBe('%Y.%m.%d');
  });

  /** A non-string DATE_FORMAT is corrupt and must be skipped, not seeded as junk. */
  test('skips a non-string DATE_FORMAT', () => {
    app.seedConfigFromWatch(messageKeys, { DATE_FORMAT: 42 });

    const result = 'DATE_FORMAT' in JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toBe(false);
  });

  /** A numeric enum (the on-wire form) must be stringified to match Clay's option values. */
  test('stringifies a numeric enum value', () => {
    app.seedConfigFromWatch(messageKeys, { THEME: 3 });

    const result = stored('THEME');

    expect(result).toBe('3');
  });

  /** A non-primitive enum is malformed and must be skipped. */
  test('skips an enum value that is neither string nor number', () => {
    app.seedConfigFromWatch(messageKeys, { THEME: { nested: true } });

    const result = 'THEME' in JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toBe(false);
  });

  /** The unit must coerce to a strict boolean off the 1 sentinel, so a junk value can't seed a truthy Fahrenheit flag. */
  test.each([
    [1, true],
    [0, false],
    ['nonsense', false]
  ])('coerces TEMPERATURE_UNIT %s to %s', (value, expected) => {
    app.seedConfigFromWatch(messageKeys, { TEMPERATURE_UNIT: value });

    const result = stored('TEMPERATURE_UNIT');

    expect(result).toBe(expected);
  });

  /** A dropped field (or wrong coercion) in the seed table silently stops seeding that setting, so its config page opens on the default. */
  test('seeds every supported field from a full payload', () => {
    app.seedConfigFromWatch(messageKeys, {
      TEMPERATURE_UNIT: 1,
      DATE_FORMAT: '%a %d %b',
      THEME: 2,
      STEPS_MODE: 1,
      TIME_FORMAT: 0,
      BLUETOOTH_ICON: 0,
      BLUETOOTH_VIBE_CONNECT: 3,
      BLUETOOTH_VIBE_DISCONNECT: 1
    });

    const result = JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toEqual({
      TEMPERATURE_UNIT: true,
      DATE_FORMAT: '%a %d %b',
      THEME: '2',
      STEPS_MODE: '1',
      TIME_FORMAT: '0',
      BLUETOOTH_ICON: false,
      BLUETOOTH_VIBE_CONNECT: '3',
      BLUETOOTH_VIBE_DISCONNECT: '1'
    });
  });
});

describe('weatherSettingsSnapshot', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /** The snapshot must JSON-encode the stored weather keys so a later save can be diffed by content. */
  test('json-encodes the weather keys read from the store', () => {
    localStorage.setItem('clay-settings', JSON.stringify({ WEATHER_PROVIDER: 'owm' }));

    const result = app.weatherSettingsSnapshot();

    expect(result[app.WEATHER_KEYS.indexOf('WEATHER_PROVIDER')]).toBe('"owm"');
  });
});

describe('weatherSettingsChanged', () => {
  /** With no snapshot (the page never reported opening) the safe default is to refetch. */
  test('returns true when there is no prior snapshot', () => {
    const result = app.weatherSettingsChanged(null, ['"metric"']);

    expect(result).toBe(true);
  });

  /** Identical snapshots mean only non-weather settings changed, so no refetch. */
  test('returns false when every weather key is unchanged', () => {
    const before = app.WEATHER_KEYS.map(() => '"same"');

    const result = app.weatherSettingsChanged(before, before.slice());

    expect(result).toBe(false);
  });

  /** A single differing weather key must trigger a refetch. */
  test('returns true when a weather key differs', () => {
    const before = app.WEATHER_KEYS.map(() => '"a"');
    const after = before.slice();
    after[0] = '"b"';

    const result = app.weatherSettingsChanged(before, after);

    expect(result).toBe(true);
  });
});
