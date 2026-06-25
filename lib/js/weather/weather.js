/**
 * Weather lookup dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns
 * its result through `done` as {temperature, condition, location, ok}, already
 * formatted to the user's unit preference.
 */
'use strict';

const openMeteo = require('./providers/openmeteo');
const owm = require('./providers/owm');
const weatherApi = require('./providers/weatherapi');

// provider name -> module
const PROVIDERS = {
  openmeteo: openMeteo,
  owm: owm,
  weatherapi: weatherApi
};

/**
 * Looks up current weather using the configured provider and location.
 *
 * @param {Object} opts provider, key, place, fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetchWeather(opts, request, done) {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unrecognized provider would silently use open-meteo and hide the misconfig, so log it
    console.log(`Unknown weather provider "${opts.provider}", using open-meteo`);
  }

  (provider || openMeteo).fetch(opts, request, done);
}

module.exports = { fetchWeather };
