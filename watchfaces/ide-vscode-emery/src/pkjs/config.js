const buildConfig = require('../../../../lib/js/pkjs/config-builder');

module.exports = buildConfig({
  theme: {
    label: "Editor Theme",
    description: "Colour scheme for the editor frame and readouts.",
    options: [
      { "label": "Dark", "value": 0 },
      { "label": "Light", "value": 1 },
      { "label": "Terminal", "value": 2 },
      { "label": "Cyberpunk", "value": 3 },
      { "label": "Synthwave '84", "value": 4 },
      { "label": "Mono (Black & White)", "value": 5 }
    ]
  },
  bluetooth: {
    description: "Show a bluetooth glyph in the status bar - lit when the phone is connected, slashed when it drops."
  },  
  location: { gpsDefault: false },
  weather: {},
  temperature: {},
  date: {
    default: "%b %d",
  },
  steps: {
    label: "Stats Readout",
    description: "What the terminal steps line shows: step count, or distance walked."
  },
  battery: {
    description: "What the tab-strip battery readout shows: the icon, the percentage, or both."
  }
});
