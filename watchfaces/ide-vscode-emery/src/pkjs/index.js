/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/js/pkjs/app.js). This face
 * shows the weather condition and temperature but no coordinates, so
 * formatCoords sends no location keys; everything else is shared.
 */

const { startPebbleApp } = require('../../../../lib/js/pkjs/app');
const clayConfig = require('./config');

startPebbleApp({
  clayConfig,
  // this face never displays coordinates, so it sends none
  formatCoords: () => ({})
});
