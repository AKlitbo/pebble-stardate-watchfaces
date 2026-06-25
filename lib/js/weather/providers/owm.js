/**
 * OpenWeatherMap weather provider (requires an API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.js), so this module never geocodes.
 */
'use strict';

const util = require('../util');

const OWM_WEATHER_API = 'https://api.openweathermap.org/data/2.5/weather';

/**
 * Fetches current weather from OpenWeatherMap for the supplied coordinates.
 *
 * @param {Object} opts key, fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetch(opts, request, done) {
  if (!opts.key) {
    return done(util.status('No API Key'));
  }

  if (!opts.coords) {
    return done(util.status('No Location'));
  }

  const units = opts.fahrenheit ? 'imperial' : 'metric';
  const encodedKey = encodeURIComponent(opts.key);
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  const url = `${OWM_WEATHER_API}?lat=${lat}&lon=${lon}&units=${units}&appid=${encodedKey}`;

  util.requestJson(url, request, done, (json) => {
    // OWM returns a 'cod' field with the HTTP status - 401 is usually a bad key
    // it arrives as a number on success but often a string on errors, so normalize before comparing
    const cod = Number(json.cod);
    if (Number.isFinite(cod) && cod !== 200) {
      console.log('owm api error:', json.message);
      return done(util.status(cod === 401 ? 'Invalid Key' : 'API Error'));
    }

    if (!json.main || !json.weather || !json.weather.length) {
      return done(util.status('No Wx Data'));
    }

    // OWM icon codes end in 'd' (day) or 'n' (night) - a missing icon is
    // treated as day so a clear sky never wrongly shows a moon
    const icon = json.weather[0].icon || '';
    const isDay = !icon.endsWith('n');

    done(util.ok(
      json.main.temp,
      util.applyNight(json.weather[0].main, isDay),
      opts.label || json.name,
      lat,
      lon
    ));
  });
}

module.exports = { fetch };
