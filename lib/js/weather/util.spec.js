/**
 * Specs for the shared weather helpers.
 *
 * These are pure functions that turn upstream data into what the watchface
 * renders. A wrong threshold, alias, or truncation shows the user the wrong
 * weather word or breaks the layout, so they are tested exhaustively.
 */

import { describe, test, expect, vi } from 'vitest';
import util from './util.js';
import conditions from './conditions.js';

describe('wmoToCondition', () => {
  /** A wrong code boundary shows the wrong sky word (e.g. snow reported as rain). */
  test.each([
    [0, 'CLEAR'],
    [1, 'CLDY'],
    [3, 'CLDY'],
    [48, 'FOGGY'],
    [51, 'DRZL'],
    [55, 'DRZL'],
    [57, 'FZDZ'],
    [61, 'RAIN'],
    [65, 'RAIN'],
    [67, 'FZRN'],
    [71, 'SNOW'],
    [75, 'SNOW'],
    [77, 'SNOW'],
    [80, 'SHWR'],
    [82, 'SHWR'],
    [86, 'SNSH'],
    [95, 'STRM'],
    [99, 'STRM']
  ])('maps WMO code %i to %s', (code, expected) => {
    const result = util.wmoToCondition(code);

    expect(result).toBe(expected);
  });

  /** An out-of-range code must fall back to a safe word, never render undefined. */
  test('maps a code above the known range to Unknown', () => {
    const result = util.wmoToCondition(100);

    expect(result).toBe('UNKNOWN');
  });
});

describe('condition vocabulary single source of truth', () => {
  /**
   * Every token the JS producer can emit must exist in conditions.js, which
   * generates the C icon table. A token the C side lacks renders WI_NA silently,
   * so this guards the two sides from drifting (UNKNOWN is the deliberate fallback).
   */
  test('every producible token is in conditions.js (or the UNKNOWN fallback)', () => {
    const known = new Set(conditions.conditions.map((entry) => entry.token));

    const emitted = new Set();
    for (let code = 0; code <= 99; code++) {
      emitted.add(util.wmoToCondition(code));
    }
    emitted.add(util.applyNight('CLEAR', false));

    const unmapped = [...emitted].filter((token) => token !== 'UNKNOWN' && !known.has(token));

    expect(unmapped).toEqual([]);
  });
});

describe('applyNight', () => {
  /** A clear sky at night must become its night variant so the watch shows a moon, not a sun. */
  test('promotes Clear to Clear Night when it is not day', () => {
    const result = util.applyNight('Clear', false);

    expect(result).toBe('CLEAR_NIGHT');
  });

  /** A clear sky by day must stay Clear so the watch shows the sun. */
  test('leaves Clear unchanged during the day', () => {
    const result = util.applyNight('Clear', true);

    expect(result).toBe('CLEAR');
  });

  /** Only Clear has a night form - every other condition passes through untouched. */
  test.each([
    ['Cloudy', false],
    ['Rain', false],
    ['Snow', false],
    ['T-Storm', false]
  ])('leaves %s unchanged at night', (condition, isDay) => {
    const result = util.applyNight(condition, isDay);

    expect(result).toBe(condition.toUpperCase());
  });
});

describe('shorten', () => {
  /** A verbose, un-aliased phrase must map to UNKNOWN, never a truncated fragment -
   *  a partial word the C icon table can't match would render the wrong/NA icon. */
  test('maps an unrecognized phrase to Unknown instead of truncating', () => {
    const result = util.shorten('Moderate or heavy freezing rain');

    expect(result).toBe('UNKNOWN');
  });

  /** An 11-char string sits exactly at the limit and must pass through untouched. */
  test('leaves an 11-char string unchanged', () => {
    const result = util.shorten('CLEAR_NIGHT');

    expect(result).toBe('CLEAR_NIGHT');
  });

  /** Without aliasing, verbose provider phrases overflow the watch field. */
  test.each([
    ['Sunny', 'CLEAR'],
    ['Partly cloudy', 'CLDY'],
    ['Patchy rain nearby', 'RAIN'],
    ['Thunderstorm', 'STRM'],
    ['Light snow', 'SNOW'],
    ['Mist', 'FOGGY']
  ])('aliases %s to %s', (input, expected) => {
    const result = util.shorten(input);

    expect(result).toBe(expected);
  });

  /** A null/empty condition must render a safe word, never blank or "undefined". */
  test.each([[''], [null], [undefined]])('returns Unknown for falsy input (%s)', (input) => {
    const result = util.shorten(input);

    expect(result).toBe('UNKNOWN');
  });
});

describe('safeParse', () => {
  /** A valid body must parse so the reading can be shown. */
  test('parses valid JSON into an object', () => {
    const result = util.safeParse('{"temp":12}');

    expect(result).toEqual({ temp: 12 });
  });

  /** A malformed body must yield null (not throw) so the caller can show an error state. */
  test('returns null for malformed JSON instead of throwing', () => {
    const result = util.safeParse('not json');

    expect(result).toBeNull();
  });
});

describe('requestJson', () => {
  /** A structured error body (e.g. a provider's 401 JSON) must still reach the provider so it can classify it, not be swallowed as a network error. */
  test('hands a parseable body to onJson even when the request reported an http error', () => {
    const request = (url, callback) => callback('http 401', '{"cod":401}');
    const onJson = vi.fn();

    util.requestJson('https://example', request, () => {}, onJson);

    expect(onJson).toHaveBeenCalledWith({ cod: 401 });
  });

  /** A failed request whose body cannot be parsed must become a NET ERROR status, never a fake reading. */
  test('returns a NET ERROR status when the request failed and the body is unparseable', () => {
    const request = (url, callback) => callback('http 503', '<html>service down</html>');
    let result;

    util.requestJson('https://example', request, (status) => { result = status; }, () => {});

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('NET ERROR');
  });

  /** A 2xx whose body is not JSON must surface as BAD WX DATA rather than success. */
  test('returns a BAD WX DATA status when a successful body is unparseable', () => {
    const request = (url, callback) => callback(null, 'not json');
    let result;

    util.requestJson('https://example', request, (status) => { result = status; }, () => {});

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('BAD WX DATA');
  });

  /** A clean success must parse and route to the provider. */
  test('hands a parseable success body to onJson', () => {
    const request = (url, callback) => callback(null, '{"temp":12}');
    const onJson = vi.fn();

    util.requestJson('https://example', request, () => {}, onJson);

    expect(onJson).toHaveBeenCalledWith({ temp: 12 });
  });
});

describe('ok', () => {
  /** The watch renders an integer, since an unrounded float would overflow the field. */
  test('rounds the temperature and marks the result ok', () => {
    const result = util.ok(13.6, 'Clear', 'Town');

    expect(result).toEqual({
      temperature: 14,
      condition: 'CLEAR',
      location: 'Town',
      ok: true
    });
  });

  /** A provider response missing the temperature must not ship NaN as a real reading. */
  test('returns a No Wx Data status when the temperature is not finite', () => {
    const result = util.ok(undefined, 'Clear', 'Town');

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('NO WX DATA');
  });

  /** The condition must be passed through shorten so long phrases never reach the watch. */
  test('shortens the condition text', () => {
    const result = util.ok(10, 'Light rain', 'Town');

    expect(result.condition).toBe('RAIN');
  });

  /** A null location must become '' so the watch never renders the word "null". */
  test('falls back to an empty location label', () => {
    const result = util.ok(10, 'Clear', null);

    expect(result.location).toBe('');
  });

  /** The resolved coordinates must ride along when both are real, so the watch can show lat/lon. */
  test('attaches lat/lon when both are numbers', () => {
    const result = util.ok(10, 'Clear', 'Town', 51.5, -0.12);

    expect(result.lat).toBe(51.5);
    expect(result.lon).toBe(-0.12);
  });

  /** Without both coordinates the watch must show no lat/lon, never a phantom 0,0. */
  test.each([
    [undefined, undefined],
    [51.5, undefined],
    [undefined, -0.12]
  ])('omits lat/lon unless both are numbers (lat=%s, lon=%s)', (lat, lon) => {
    const result = util.ok(10, 'Clear', 'Town', lat, lon);

    expect(result).not.toHaveProperty('lat');
    expect(result).not.toHaveProperty('lon');
  });
});
