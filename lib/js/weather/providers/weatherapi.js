/**
 * WeatherAPI.com weather provider (requires an API key).
 *
 * WeatherAPI resolves a place name or "lat,lon" query server-side, so this
 * module needs no separate geocoding step.
 */
'use strict';

const util = require('../util');

const WEATHER_API_URL = 'https://api.weatherapi.com/v1/current.json';

/**
 * Fetches current weather from WeatherAPI.com.
 *
 * @param {Object} opts place, key, fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetch(opts, request, done) {
  if (!opts.key) {
    return done(util.status('No API Key'));
  }

  // WeatherAPI accepts a "lat,lon" string or a place name in the same q param
  const query = opts.coords
    ? `${opts.coords.lat},${opts.coords.lon}`
    : opts.place;

  if (!query) {
    return done(util.status('No Location'));
  }

  const url = `${WEATHER_API_URL}?key=${encodeURIComponent(opts.key)}&q=${encodeURIComponent(query)}`;

  util.requestJson(url, request, done, (json) => {
    // WeatherAPI signals failures with an error object carrying a numeric code
    if (json.error) {
      console.log('weatherapi error:', json.error.message);

      // 1006 is 'no matching location found'
      if (json.error.code === 1006) {
        return done(util.status('Loc Not Found'));
      }

      // 1002, 2006, 2008 are various forms of invalid/disabled API keys
      if ([1002, 2006, 2008].includes(json.error.code)) {
        return done(util.status('Invalid Key'));
      }

      return done(util.status('API Error'));
    }

    if (!json.current || !json.current.condition) {
      return done(util.status('No Wx Data'));
    }

    // prefer the label resolved at geocode time, then WeatherAPI's own
    // resolved name, then the raw place the user typed
    const label = opts.label ||
        (json.location && json.location.name) ||
        (opts.coords ? 'My Location' : opts.place);

    const temperature = opts.fahrenheit ? json.current.temp_f : json.current.temp_c;

    // is_day is 1 by day and 0 at night - treat a missing value as day so a
    // clear sky never wrongly shows a moon
    const isDay = json.current.is_day !== 0;

    // WeatherAPI echoes the resolved location's coordinates in json.location
    const loc = json.location || {};

    done(util.ok(
      temperature,
      util.applyNight(json.current.condition.text, isDay),
      label,
      loc.lat,
      loc.lon
    ));
  });
}

module.exports = { fetch };
