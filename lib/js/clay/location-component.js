/**
 * Clay custom component for live location autocomplete.
 *
 * As the user types, it queries Open-Meteo's free geocoding API and shows
 * matching places. Selecting one stores the resolved coordinates so the watch
 * never has to geocode.
 *
 * Persisted value:
 *   JSON string {lat, lon, label}
 *
 * IMPORTANT: `initialize` and the `manipulator` methods are serialised
 * (via .toString()) and run inside the Clay config webview, a separate JS
 * context. They must be self-contained and must NOT reference anything from
 * this module's scope (module-level constants, requires, etc.) - those are
 * undefined in the webview and will throw. Keep such values inline.
 */

module.exports = {
  name: 'locationsearch',

  template: [
    '<div class="component component-input loc-search">',
    '  <label class="tap-highlight">',
    '    <span class="label loc-label"></span>',
    '    <span class="input">',
    '      <input type="text" class="loc-query" autocomplete="off" autocorrect="off" autocapitalize="words">',
    '      <ul class="loc-list"></ul>',
    '    </span>',
    '  </label>',
    '  <input type="hidden" class="loc-value">',
    '</div>'
  ].join(''),

  style: [
    '.loc-search .input { position: relative; }',
    '.loc-search .loc-list { position: absolute; left: 0; right: 0; top: 100%; z-index: 10; list-style: none; margin: 0; padding: 0; background: #fff; color: #000; border: 1px solid #ccc; border-radius: 0 0 4px 4px; max-height: 180px; overflow-y: auto; }',
    '.loc-search .loc-list:not(.show) { display: none; }',
    '.loc-search .loc-item { padding: 10px 12px; cursor: pointer; border-bottom: 1px solid #eee; }',
    '.loc-search .loc-item:last-child { border-bottom: none; }',
    '.loc-search .loc-item:hover, .loc-search .loc-item:active { background: #ff4700; color: #fff; }'
  ].join(''),

  manipulator: {
    /**
     * Restores a previously saved selection.
     * 
     * Value is a JSON string {lat, lon, label}, or ''.
     * 
     * @param {string} value The persisted JSON string.
     */
    set: function(value) {
      var root = this.$element[0];
      var label = '';

      root.querySelector('.loc-value').value = value || '';

      try {
        var saved = value ? JSON.parse(value) : null;
        if (saved && saved.label) {
          label = saved.label;
        }
      } catch (error) {
        // leave the box empty on bad data
      }

      root.querySelector('.loc-query').value = label;
    },

    /**
     * Returns the JSON string for the chosen location.
     * 
     * @return {string} The JSON string or ''.
     */
    get: function() {
      return this.$element[0].querySelector('.loc-value').value || '';
    }
  },

  /**
   * Initializes the component within the Clay webview.
   */
  initialize: function() {
    var self = this;
    var root = self.$element[0];

    var queryEl = root.querySelector('.loc-query');
    var hiddenEl = root.querySelector('.loc-value');
    var listEl = root.querySelector('.loc-list');
    var timer = null;
    var seq = 0;

    root.querySelector('.loc-label').textContent = self.config.label || 'Location';
    if (self.config.attributes && self.config.attributes.placeholder) {
      queryEl.placeholder = self.config.attributes.placeholder;
    }

    /**
     * Formats a place object into a display label.
     * 
     * @param {Object} place The place object.
     * @return {string} The formatted label.
     */
    function labelFor(place) {
      return [place.name, place.admin1, place.country].filter(Boolean).join(', ');
    }

    /**
     * Hides the suggestions list and clears its contents.
     */
    function hideList() {
      while (listEl.firstChild) {
        listEl.removeChild(listEl.firstChild);
      }

      listEl.classList.remove('show');
    }

    /**
     * Renders the list of geocoding results.
     * 
     * Builds suggestions using DOM nodes rather than innerHTML to prevent markup injection.
     * 
     * @param {Array} results The list of places.
     */
    function renderResults(results) {
      hideList();

      if (!results || !results.length) {
        return;
      }

      results.forEach(function(place) {
        var item = document.createElement('li');
        item.className = 'loc-item';
        item.textContent = labelFor(place);
        item.addEventListener('click', function(event) {
          event.stopPropagation(); // stop the document click listener from dismissing the list first
          self.set(JSON.stringify({ lat: place.latitude, lon: place.longitude, label: labelFor(place) }));
          hideList();
        });

        listEl.appendChild(item);
      });

      listEl.classList.add('show');
    }

    /**
     * Executes a geocoding search query.
     * 
     * @param {string} query The search string.
     */
    function search(query) {
      var mySeq = ++seq;
      var xhr = new XMLHttpRequest();
      // inlined, not a module constant: this runs in the serialised webview
      // context where module scope is unavailable (see file header)
      var url = 'https://geocoding-api.open-meteo.com/v1/search?name=' +
        encodeURIComponent(query) + '&count=5&language=en&format=json';

      xhr.onload = function() {
        // ignore a stale response that lands after a newer query
        if (mySeq !== seq) {
          return;
        }

        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            var data = JSON.parse(xhr.responseText);
            renderResults(data && data.results);
          } catch (error) {
            hideList();
          }
        } else {
          hideList();
        }
      };

      // a network failure or timeout leaves no suggestions rather than a stale list
      xhr.onerror = function() {
        if (mySeq === seq) {
          hideList();
        }
      };
      xhr.ontimeout = function() {
        if (mySeq === seq) {
          hideList();
        }
      };
      xhr.timeout = 10000;

      xhr.open('GET', url);
      xhr.send();
    }

    // typing clears the chosen coordinates and debounces the geocoding calls
    queryEl.addEventListener('input', function() {
      hiddenEl.value = '';
      var query = (queryEl.value || '').trim();

      if (timer) {
        clearTimeout(timer);
      }

      if (query.length < 2) {
        seq++; // invalidate any in-flight response so it can't repopulate the list
        return hideList();
      }

      timer = setTimeout(function() {
        search(query);
      }, 300);
    });

    document.addEventListener('click', function(event) {
      if (!root.contains(event.target)) {
        hideList();
      }
    });
  }
};