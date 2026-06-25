/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/js/pkjs/app.js). This face
 * formats coordinates as a single hemisphere-style readout in the LATITUDE key;
 * everything else is shared.
 */

const { startPebbleApp } = require('../../../../lib/js/pkjs/app');
const clayConfig = require('./config');

/**
 * Formats one signed coordinate as a magnitude plus hemisphere letter.
 * E.g. 34.0522 -> "34.05N", -118.2437 -> "118.24W".
 * @param {*} value
 * @param {string} positive Hemisphere letter when value >= 0.
 * @param {string} negative Hemisphere letter when value < 0.
 * @return {string}
 */
function fmtCoord(value, positive, negative) {
  if (typeof value !== 'number' || Number.isNaN(value)) {
    return '';
  }

  const hemisphere = value >= 0 ? positive : negative;
  return Math.abs(value).toFixed(2) + hemisphere;
}

/**
 * Builds the combined "lat lon" coordinate readout, or a no-fix placeholder.
 * @param {!Object} result
 * @return {string}
 */
function coordString(result) {
  if (typeof result.lat === 'number' && typeof result.lon === 'number') {
    return fmtCoord(result.lat, 'N', 'S') + ' ' + fmtCoord(result.lon, 'E', 'W');
  }

  return 'NO LOCK';
}

startPebbleApp({
  clayConfig,
  // the combined readout travels in LATITUDE; LONGITUDE is left empty
  formatCoords: (messageKeys, result) => ({
    [messageKeys.LATITUDE]: coordString(result),
    [messageKeys.LONGITUDE]: ''
  })
});
