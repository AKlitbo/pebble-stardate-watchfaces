import os
import shutil
import json
import glob


def _repo_root(ctx):
    """
    The repo root - the nearest ancestor of the face dir holding both watchfaces/
    and lib/. Derived by walking up rather than assuming a fixed depth, so the
    layout can move without breaking every wscript.
    """
    node = ctx.path
    while node is not None:
        if node.find_dir('watchfaces') and node.find_dir('lib'):
            return node
        node = node.parent

    ctx.fatal('waf_helpers: could not locate the repo root (no ancestor with watchfaces/ and lib/)')


def generate_conditions(ctx):
    """
    Regenerate the weather-icon lookup (icons_table.h) from the shared condition
    vocabulary in lib/js/weather/conditions.js. Non-fatal: the generated header is
    committed, so a node-less environment still builds with the last-generated table.
    """
    from waflib import Logs

    script = _repo_root(ctx).find_node('tools/generate-conditions.js')
    if script and ctx.exec_command(['node', script.abspath()]) != 0:
        Logs.warn('generate-conditions: node unavailable; using committed icons_table.h')


def generate_frames(ctx):
    """
    Bake each face's HTML frame chrome into its background PNGs by invoking the
    shared tools/generate-frame.js.

    Incremental: a face is re-baked only when a frame source (frame/**/*.html or
    *.css) is newer than its background*.png, so an untouched build launches no
    browser. Non-fatal: the PNGs are committed, so a build without node/playwright
    falls back to the last-baked images.

    Themes are discovered dynamically. A single-frame face bakes every
    frame/css/theme_*.css via "--theme all" (the generator enumerates them); a
    multi-frame face bakes each frame/*.html, which links its own palette.
    """
    from waflib import Logs

    face = ctx.path
    tool = _repo_root(ctx).find_node('tools/generate-frame.js')
    frame_dir = face.find_dir('frame')
    if not tool or not frame_dir:
        return

    htmls = frame_dir.ant_glob('*.html')
    if not htmls or not _frames_stale(face, frame_dir):
        return

    # one frame -> bake every theme palette; many frames -> bake each in turn
    commands = []
    if len(htmls) > 1:
        for html in htmls:
            commands.append(['node', tool.abspath(), html.name])
    else:
        command = ['node', tool.abspath(), htmls[0].name]
        if frame_dir.ant_glob('css/theme_*.css'):
            command += ['--theme', 'all']
        commands.append(command)

    for command in commands:
        if ctx.exec_command(command) != 0:
            Logs.warn('generate-frame: node/playwright unavailable; using committed background PNGs')
            return


def _frames_stale(face, frame_dir):
    """True when a frame source is newer than the baked PNGs, or none exist yet."""
    images_dir = face.find_dir('resources/images')
    images = images_dir.ant_glob('background*.png') if images_dir else []
    if not images:
        return True

    sources = frame_dir.ant_glob('**/*.html') + frame_dir.ant_glob('**/*.css')
    if not sources:
        return False

    newest_source = max(os.path.getmtime(node.abspath()) for node in sources)
    oldest_output = min(os.path.getmtime(node.abspath()) for node in images)
    return newest_source > oldest_output


def build_face(ctx, exclude_c=None):
    """
    The shared build for every face: resolve include paths, collect this face's
    C/JS plus the shared lib/, compile the app (and optional worker) per target
    platform, bundle with PebbleKit JS, and archive the .pbw afterward.

    exclude_c is an ant-glob exclude list for the shared lib/c sweep - a face with
    no weather glyph passes '**/weather/icons.c' to drop the icon resources it
    doesn't bundle.
    """
    repo = _repo_root(ctx)

    # lib/ = code shared by ALL faces. lib/c/ is pure (SDK-free, host-testable);
    # lib/c/pebble/ needs the SDK; lib/js/ is PebbleKit JS
    lib_dir = repo.find_dir('lib')
    if not lib_dir:
        ctx.fatal('Could not find the "lib" directory at the repo root.')

    lib_c = lib_dir.find_dir('c')
    lib_c_pebble = lib_c.find_dir('pebble')
    lib_js = lib_dir.find_dir('js')
    # this face's own sources (zone table, widgets, main, any skin code)
    local_c = ctx.path.find_dir('src/c')

    include_paths = [
        lib_c.abspath(),
        lib_c.find_dir('clock').abspath(),
        lib_c.find_dir('units').abspath(),
        lib_c.find_dir('text').abspath(),
        lib_c_pebble.abspath(),
        lib_c_pebble.find_dir('appmessage').abspath(),
        lib_c_pebble.find_dir('settings').abspath(),
        lib_c_pebble.find_dir('weather').abspath(),
        lib_c_pebble.find_dir('units').abspath(),
        lib_c_pebble.find_dir('health').abspath(),
        lib_c_pebble.find_dir('shell').abspath(),
        lib_c_pebble.find_dir('ui').abspath(),
        local_c.abspath()
    ]

    # one glob recurses into both lib/c/ (pure) and lib/c/pebble/ (SDK)
    c_sources = (ctx.path.ant_glob('src/c/**/*.c')
                 + lib_c.ant_glob('**/*.c', excl=exclude_c or []))

    js_sources = ctx.path.ant_glob(['src/pkjs/**/*.js', 'src/pkjs/**/*.json'])
    if lib_js:
        # exclude *.spec.js - test files, never imported by index.js, not shipped
        js_sources += lib_js.ant_glob('**/*.js', excl=['**/*.spec.js'])

    build_worker = os.path.exists('worker_src')
    binaries = []
    cached_env = ctx.env

    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)

        ctx.env.append_value('INCLUDES', include_paths)

        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)

        ctx.pbl_build(
            source=c_sources,
            target=app_elf,
            bin_type='app'
        )

        binary_info = {'platform': platform, 'app_elf': app_elf}

        if build_worker:
            worker_elf = '{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            ctx.pbl_build(
                source=ctx.path.ant_glob('worker_src/c/**/*.c'),
                target=worker_elf,
                bin_type='worker'
            )
            binary_info['worker_elf'] = worker_elf

        binaries.append(binary_info)

    ctx.env = cached_env

    ctx.set_group('bundle')
    ctx.pbl_bundle(
        binaries=binaries,
        js=js_sources,
        js_entry_file='src/pkjs/index.js'
    )

    ctx.add_post_fun(copy_current_pbw)


def copy_current_pbw(ctx):
    """
    After the pebble bundle is built, this copies the resulting .pbw into the
    releases/<face> directory, appending the version number from package.json.
    Any older .pbw versions are moved to a 'previous' subdirectory.
    """
    face = ctx.path.name

    src_pbw = os.path.join(ctx.path.abspath(), 'build', '{}.pbw'.format(face))
    if not os.path.exists(src_pbw):
        return

    releases_dir = os.path.join(_repo_root(ctx).abspath(), 'releases', face)
    if not os.path.isdir(releases_dir):
        os.makedirs(releases_dir)

    pkg_path = os.path.join(ctx.path.abspath(), 'package.json')
    if os.path.exists(pkg_path):
        # a malformed package.json should fail with a clear message, not a traceback
        version = 'unknown'
        try:
            with open(pkg_path, 'r') as pkg_f:
                version = json.load(pkg_f).get('version', 'unknown')
        except (ValueError, OSError) as err:
            ctx.fatal('waf_helpers: could not read {}: {}'.format(pkg_path, err))

        dest_filename = '{}-{}.pbw'.format(face, version)
        dest_file = os.path.join(releases_dir, dest_filename)

        previous_dir = os.path.join(releases_dir, 'previous')
        for existing_pbw in glob.glob(os.path.join(releases_dir, '*.pbw')):
            if os.path.basename(existing_pbw) != dest_filename:
                if not os.path.isdir(previous_dir):
                    os.makedirs(previous_dir)
                shutil.move(existing_pbw, os.path.join(previous_dir, os.path.basename(existing_pbw)))

        shutil.copy2(src_pbw, dest_file)
