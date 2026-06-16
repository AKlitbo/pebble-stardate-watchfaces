/*
 * index.js - PebbleKit JS entry point.
 *
 * Reads the Clay settings, decides the location source (phone GPS or the
 * configured city), fetches the weather through the selected provider module,
 * and forwards it to the watch over AppMessage.
 */

// using the internal bundle which is compatible with Pebble's webpack
const Clay = require('@rebble/clay/src/js/index');
const messageKeys = require('message_keys');
const clayConfig = require('./config');
const weather = require('../../../../shared/pkjs/weather/weather');
const { status } = require('../../../../shared/pkjs/weather/util');

const clay = new Clay(clayConfig);

// register the location autocomplete (referenced as type "locationsearch" in
// config.js) before the settings page is built
clay.registerComponent(require('../../../../shared/pkjs/clay/location-component'));

/**
 * Collects {messageKey: defaultValue} from the Clay config so the same
 * defaults apply whether or not the user has opened the settings page yet.
 * @param {!Array<!Object>} items
 * @return {!Object}
 */
function collectDefaults(items) {
  return items.reduce((defaults, item) => {
    if (item.messageKey && item.defaultValue !== undefined) {
      defaults[item.messageKey] = item.defaultValue;
    }
    if (item.items) {
      Object.assign(defaults, collectDefaults(item.items));
    }
    return defaults;
  }, {});
}

/** @const {!Object} The defaults declared in config.js. */
const DEFAULTS = collectDefaults(clayConfig);

/**
 * Performs an HTTP GET. 
 * @param {string} url
 * @param {function(?string, string=)} callback callback(error, responseText)
 */
function request(url, callback) {
  const xhr = new XMLHttpRequest();

  xhr.onload = () => {
    callback(null, xhr.responseText);
  };
  
  xhr.onerror = () => {
    callback('network error');
  };
  
  xhr.ontimeout = () => {
    callback('timeout');
  };
  
  xhr.timeout = 15000;
  
  xhr.open('GET', url);
  xhr.send();
}

/**
 * Reads the persisted Clay settings from localStorage.
 * @return {!Object}
 */
function getConfig() {
  try {
    return JSON.parse(localStorage.getItem('clay-settings')) || {};
  } catch (e) {
    return {};
  }
}

/**
 * Unwraps a Clay value, applying a fallback for empty values.
 * @param {*} value
 * @param {*} fallback
 * @return {*}
 */
function readValue(value, fallback) {
  let result = value;

  if (result && typeof result === 'object' && 'value' in result) {
    result = result.value;
  }

  if (result === undefined || result === null || result === '') {
    return fallback;
  }

  return result;
}

/**
 * Reads a boolean Clay setting, applying a fallback when it is unset.
 * @param {*} value
 * @param {boolean} fallback
 * @return {boolean}
 */
function readBool(value, fallback) {
  const result = readValue(value, fallback);
  return result === true || result === 'true' || result === 1 || result === '1';
}

/**
 * Formats a decimal coordinate in LCARS dash style. 
 * E.g. 33.448376 -> "33-448", -112.074036 -> "-112-074".
 * @param {number} v
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

/**
 * Sends a weather result to the watch.
 * @param {!Object} result
 */
function sendWeather(result) {
  const dict = {
    [messageKeys.TEMPERATURE]: result.temperature,
    [messageKeys.CONDITIONS]: result.condition,
    [messageKeys.WEATHER_OK]: result.ok ? 1 : 0,
    [messageKeys.LATITUDE]: '',
    [messageKeys.LONGITUDE]: ''
  };

  if (typeof result.lat === 'number') {
    dict[messageKeys.LATITUDE] = fmtCoord(result.lat);
  }

  if (typeof result.lon === 'number') {
    dict[messageKeys.LONGITUDE] = fmtCoord(result.lon);
  }

  Pebble.sendAppMessage(
    dict,
    () => {
      console.log('Weather info sent to Pebble successfully!');
    },
    () => {
      console.error('Error sending weather info to Pebble!');
    }
  );
}

/**
 * Reads the manual location the user picked in settings.
 * @param {!Object} config
 * @return {?{coords: {lat: number, lon: number}, label: string}}
 */
function getManualLocation(config) {
  const raw = readValue(config.LOCATION_NAME, '');
  
  if (!raw) {
    return null;
  }
  
  try {
    let o = raw;
    if (typeof raw === 'string') {
      o = JSON.parse(raw);
    }
    
    if (o && typeof o.lat === 'number' && typeof o.lon === 'number') {
      return { 
        coords: { lat: o.lat, lon: o.lon }, 
        label: o.label || '' 
      };
    }
  } catch (e) { 
    // fall through on parsing error
  }
  
  return null;
}

/**
 * Fetches the weather using GPS or the stored manual coordinates, then forwards
 * the result to the watch.
 */
function getWeather() {
  const config = getConfig();
  
  const opts = {
    provider: String(readValue(config.WEATHER_PROVIDER, DEFAULTS.WEATHER_PROVIDER)),
    key: String(readValue(config.API_KEY, DEFAULTS.API_KEY || '')),
    fahrenheit: readBool(config.TEMPERATURE_UNIT, DEFAULTS.TEMPERATURE_UNIT),
    coords: null,
    label: undefined,
  };

  const useGps = readBool(config.USE_GPS, DEFAULTS.USE_GPS);
  const gpsFallback = readBool(config.GPS_FALLBACK, DEFAULTS.GPS_FALLBACK);
  const manual = getManualLocation(config);

  const fetchFor = (coords, label) => {
    opts.coords = coords;
    opts.label = label;
    console.log(`Fetching Weather: ${opts.provider} @ ${coords.lat},${coords.lon} (${label})`);
    
    weather.fetchWeather(opts, request, (result) => {
      console.log(`Weather: ${result.condition} ${result.temperature} (ok=${result.ok})`);
      sendWeather(result);
    });
  };

  const useManual = () => {
    if (manual) {
      return fetchFor(manual.coords, manual.label);
    }
    sendWeather(status('No Location'));
  };

  // skip gps entirely when the user disabled it
  if (!useGps) {
    return useManual();
  }

  // fall back to the manual location if the phone has no geolocation API
  if (typeof navigator === 'undefined' || !navigator.geolocation) {
    if (gpsFallback) {
      return useManual();
    } else {
      return sendWeather(status('No GPS'));
    }
  }

  navigator.geolocation.getCurrentPosition(
    (pos) => {
      fetchFor({ lat: pos.coords.latitude, lon: pos.coords.longitude }, 'My Location');
    },
    () => {
      if (gpsFallback) {
        useManual();
      } else {
        sendWeather(status('No GPS'));
      }
    },
    { timeout: 15000, maximumAge: 60000 }
  );
}

/**
 * Seeds the Clay store from the watch's current settings so the config opens
 * with the real values instead of defaults. The watch persist is the source of
 * truth; the phone's clay-settings can be empty or stale after an update.
 * @param {!Object} payload
 */
function seedConfigFromWatch(payload) {
  const config = getConfig();

  if (messageKeys.TEMPERATURE_UNIT in payload) {
    config.TEMPERATURE_UNIT = payload[messageKeys.TEMPERATURE_UNIT] === 1;
  }
  if (messageKeys.DATE_FORMAT in payload) {
    config.DATE_FORMAT = payload[messageKeys.DATE_FORMAT];
  }
  if (messageKeys.THEME in payload) {
    config.THEME = String(payload[messageKeys.THEME]);
  }
  if (messageKeys.TRAVERSAL_MODE in payload) {
    config.TRAVERSAL_MODE = String(payload[messageKeys.TRAVERSAL_MODE]);
  }
  if (messageKeys.TIME_FORMAT in payload) {
    config.TIME_FORMAT = String(payload[messageKeys.TIME_FORMAT]);
  }

  localStorage.setItem('clay-settings', JSON.stringify(config));
}

// app lifecycle listeners
Pebble.addEventListener('ready', () => {
  console.log('PebbleKit JS Ready!');
  Pebble.sendAppMessage({ [messageKeys.REQUEST_SETTINGS]: 1 });
  getWeather();
});

Pebble.addEventListener('appmessage', (event) => {
  const payload = event.payload || {};

  if (payload[messageKeys.REQUEST_WEATHER]) {
    getWeather();
  }

  // the watch's reply to our REQUEST_SETTINGS carries the display settings
  if (messageKeys.DATE_FORMAT in payload || messageKeys.THEME in payload) {
    seedConfigFromWatch(payload);
  }
});

Pebble.addEventListener('webviewclosed', (event) => {
  if (event && event.response) {
    setTimeout(getWeather, 250);
  }
});