/**
 * Open-Meteo weather provider (free, no API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.js), so this module never geocodes.
 */
'use strict';

const util = require('../util');

const OPEN_METEO_FORECAST_API = 'https://api.open-meteo.com/v1/forecast';

/**
 * Fetches current weather from Open-Meteo for the supplied coordinates.
 *
 * @param {Object} opts fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetch(opts, request, done) {
  if (!opts.coords) {
    return done(util.status('No Location'));
  }

  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  const url = `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=temperature_2m,weather_code,is_day&temperature_unit=${unit}`;

  util.requestJson(url, request, done, (json) => {
    if (json.error) {
      console.log('open-meteo api error:', json.reason);
      return done(util.status('API Error'));
    }

    if (!json.current) {
      return done(util.status('No Wx Data'));
    }

    // is_day is 1 by day and 0 at night - treat a missing value as day so a
    // clear sky never wrongly shows a moon
    const isDay = json.current.is_day !== 0;

    done(util.ok(
      json.current.temperature_2m,
      util.applyNight(util.wmoToCondition(json.current.weather_code), isDay),
      opts.label || 'My Location',
      lat,
      lon
    ));
  });
}

module.exports = { fetch };
