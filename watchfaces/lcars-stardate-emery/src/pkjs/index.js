/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/js/pkjs/app.js). This face
 * formats coordinates in LCARS dash style into separate latitude/longitude
 * keys; everything else is shared.
 */

const { startPebbleApp } = require('../../../../lib/js/pkjs/app');
const clayConfig = require('./config');

/**
 * Formats a decimal coordinate in LCARS dash style.
 * E.g. 33.448376 -> "33-448", -112.074036 -> "-112-074".
 * @param {*} v
 * @return {string}
 */
function fmtCoord(v) {
  if (typeof v !== 'number' || Number.isNaN(v)) {
    return '';
  }

  let prefix = '';
  if (v < 0) {
    prefix = '-';
  }

  return prefix + Math.abs(v).toFixed(3).replace('.', '-');
}

startPebbleApp({
  clayConfig,
  // dash style into two keys; fmtCoord yields '' for a missing coordinate
  formatCoords: (messageKeys, result) => ({
    [messageKeys.LATITUDE]: fmtCoord(result.lat),
    [messageKeys.LONGITUDE]: fmtCoord(result.lon)
  })
});
