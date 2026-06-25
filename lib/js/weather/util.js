/**
 * Shared helpers for the weather provider modules.
 *
 * Provides the parsing, formatting, and result builders used by openmeteo,
 * owm, and weatherapi so every provider returns the same normalized shape.
 */
'use strict';

const conditions = require('./conditions');

// the tokens the C icon table recognizes (the single source of truth, see conditions.js)
const KNOWN_TOKENS = new Set(conditions.conditions.map((entry) => entry.token));

/**
 * Maps an Open-Meteo WMO weather code to a short condition string.
 *
 * @param {number} code
 * @return {string}
 */
function wmoToCondition(code) {
  if (code === 0) { return 'CLEAR'; }
  if (code <= 3) { return 'CLDY'; }
  if (code <= 48) { return 'FOGGY'; }
  if (code <= 55) { return 'DRZL'; }
  if (code <= 57) { return 'FZDZ'; }
  if (code <= 65) { return 'RAIN'; }
  if (code <= 67) { return 'FZRN'; }
  if (code <= 77) { return 'SNOW'; }
  if (code <= 82) { return 'SHWR'; }
  if (code <= 86) { return 'SNSH'; }
  if (code <= 99) { return 'STRM'; }

  return 'UNKNOWN';
}

/**
 * Promotes a clear sky to its night variant.
 *
 * So the watch can show a moon instead of a sun after dark. Only "CLEAR"
 * has a night form - every other condition is returned unchanged.
 *
 * @param {string} condition A normalized condition word (see wmoToCondition).
 * @param {boolean} isDay True for daytime, false for night.
 * @return {string}
 */
function applyNight(condition, isDay) {
  const normalized = (condition || '').toUpperCase();

  if (!isDay && normalized === 'CLEAR') {
    return 'CLEAR_NIGHT';
  }

  return normalized;
}

// provider phrases that aren't our tokens, mapped to the closest one
const CONDITION_ALIASES = {
  'SUNNY': 'CLEAR',
  'PARTLY CLOUDY': 'CLDY',
  'PATCHY RAIN POSSIBLE': 'RAIN',
  'PATCHY RAIN NEARBY': 'RAIN',
  'THUNDERSTORM': 'STRM',
  'THUNDERSTORMS': 'STRM',
  'LIGHT RAIN': 'RAIN',
  'HEAVY RAIN': 'RAIN',
  'LIGHT SNOW': 'SNOW',
  'HEAVY SNOW': 'SNOW',
  'CLOUDS': 'CLDY',
  'MIST': 'FOGGY',
  'HAZE': 'FOGGY',
  'SMOKE': 'FOGGY'
};

/**
 * Shortens a condition string so it fits beside the temperature.
 *
 * @param {string} text
 * @return {string}
 */
function shorten(text) {
  if (!text) {
    return 'UNKNOWN';
  }

  const upper = text.toUpperCase();
  if (CONDITION_ALIASES[upper]) {
    return CONDITION_ALIASES[upper];
  }
  if (KNOWN_TOKENS.has(upper)) {
    return upper;
  }

  // an unrecognized phrase (e.g. a verbose WeatherAPI string) would truncate to a
  // partial word the C icon table can't match, rendering the wrong icon - emit
  // UNKNOWN (-> WI_NA) instead of a fragment
  return 'UNKNOWN';
}

/**
 * Parses a JSON string, returning null on failure.
 *
 * @param {string} body
 * @return {?Object}
 */
function safeParse(body) {
  try {
    return JSON.parse(body);
  } catch (error) {
    return null;
  }
}

/**
 * Performs an HTTP GET and shared response handling.
 *
 * A parseable body always goes to onJson, so a provider can read its own error
 * JSON (e.g. an 'Invalid Key' on a 4xx). If the body does not parse, finish with
 * 'NET ERROR' when the request failed, else 'BAD WX DATA'.
 *
 * @param {string} url
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the finished result
 * @param {function} onJson receives the parsed JSON object
 */
function requestJson(url, request, done, onJson) {
  // cachebust
  const separator = url.indexOf('?') === -1 ? '?' : '&';
  const freshUrl = `${url}${separator}_=${Date.now()}`;

  request(freshUrl, (err, body) => {
    const json = body ? safeParse(body) : null;
    if (json) {
      return onJson(json);
    }

    if (err) {
      return done(status('NET ERROR'));
    }

    return done(status('BAD WX DATA'));
  });
}

/**
 * Builds a successful weather result.
 *
 * The temperature is already in the unit the user selected, so the watch
 * displays it without converting.
 *
 * @param {number} temperature Temperature in the user's chosen unit.
 * @param {string} condition
 * @param {?string} location Display label for the resolved location.
 * @param {number=} lat Latitude the weather was fetched for (GPS or geocoded).
 * @param {number=} lon Longitude the weather was fetched for.
 * @return {!Object}
 */
function ok(temperature, condition, location, lat, lon) {
  const value = Number(temperature);
  if (!Number.isFinite(value)) {
    // a missing or non-numeric provider field would otherwise round to NaN and ship as a real reading
    return status('No Wx Data');
  }

  const result = {
    temperature: Math.round(value),
    condition: shorten(condition),
    location: location || '',
    ok: true
  };

  // carry the resolved coordinates so the watch can show lat/lon for whatever
  // location the weather actually used (manual place name included), not just
  // the phone GPS path. Only attached when both are real numbers
  if (typeof lat === 'number' && typeof lon === 'number') {
    result.lat = lat;
    result.lon = lon;
  }

  return result;
}

/**
 * Builds a status/error result that carries no live reading.
 *
 * @param {string} text Short message shown on the watch.
 * @return {!Object}
 */
function status(text) {
  return {
    temperature: 0,
    condition: (text || '').toUpperCase(),
    location: '',
    ok: false
  };
}

module.exports = {
  wmoToCondition,
  applyNight,
  shorten,
  safeParse,
  requestJson,
  ok,
  status
};
