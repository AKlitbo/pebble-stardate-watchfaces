#!/usr/bin/env node
/*
 * generate-frame.js - bake this face's VS Code HTML chrome into background bitmaps.
 *
 * Thin wrapper over tools/generate-frame-common.js; see that for the pipeline.
 * One frame/vscode.html holds the layout and each theme is a palette-only
 * stylesheet at frame/css/theme_<name>.css.
 *
 * This chrome is axis-aligned and authored 1:1 at 200x228, so it bakes at true
 * scale (defaultScale 1) - crisp rectangles and dashed borders, pixel-faithful to
 * the HTML (Firefox antialiases the Codicon glyphs at target size). Bump with
 * --scale <n> if a future curved element needs supersampling.
 *
 * The build bakes every theme automatically; run gen:themes by hand to re-bake
 * during design:
 *   npm run gen:themes     (every theme)
 */
'use strict';

const { run } = require('../../../tools/generate-frame-common');

run({
  toolsDir: __dirname,
  defaultFrame: 'vscode',
  defaultScale: 1,
  supportsTheme: true,
  bareBackgroundBase: null,
  clearTextSelectors: ['.time', '.data'],
  hideSelectors: []
});
