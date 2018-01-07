project('Marker', 'c',
  version : '2018.01.05',
  license : 'GPL3'
)

PREFIX = get_option('prefix')
DATA_DIR = join_paths(PREFIX, 'share')
APP_DIR = join_paths(DATA_DIR, 'com.github.fabiocolacio.marker')
STYLES_DIR = join_paths(APP_DIR, 'styles')
SCRIPTS_DIR = join_paths(APP_DIR, 'scripts')
HIGHLIGHT_STYLES_DIR = join_paths(join_paths(SCRIPTS_DIR, 'highlight'),'styles')
WEB_EXTENSIONS_DIRECTORY = join_paths(APP_DIR, 'extensions')

EXECUTABLE_NAME = 'com.github.fabiocolacio.marker'

gnome = import('gnome')

res = gnome.compile_resources(
  'marker_resources',
  'src/resources/marker.gresource.xml',
  source_dir : 'src/resources'
)

marker_sources = [
  'src/marker.c',
  'src/marker-editor-window.c',
  'src/marker-exporter.c',
  'src/marker-markdown.c',
  'src/marker-prefs.c',
  'src/marker-source-view.c',
  'src/marker-preview.c',
  'src/marker-string.c',
  'src/marker-widget.c',
  res
]

webkit_extension = [
  'src/webkit-extension/scroll-extension.c'
]

hoedown_sources = [
  'src/hoedown/autolink.c',
  'src/hoedown/buffer.c',
  'src/hoedown/document.c',
  'src/hoedown/escape.c',
  'src/hoedown/html.c',
  'src/hoedown/html_blocks.c',
  'src/hoedown/html_smartypants.c',
  'src/hoedown/stack.c',
  'src/hoedown/version.c'
]

deps = [
  dependency('gtk+-3.0'),
  dependency('glib-2.0'),
  dependency('gtksourceview-3.0'),
  dependency('webkit2gtk-4.0')
]

ext_deps =[  
  dependency('webkit2gtk-4.0')
]


add_global_arguments(
  '-DSTYLES_DIR="@0@/"'.format(STYLES_DIR),
  '-DSCRIPTS_DIR="@0@/"'.format(SCRIPTS_DIR),
  '-DHIGHLIGHT_STYLES_DIR="@0@/"'.format(HIGHLIGHT_STYLES_DIR),
  '-DMARKER_VERSION="@0@"'.format(meson.project_version()),
  '-DWEB_EXTENSIONS_DIRECTORY="@0@"'.format(WEB_EXTENSIONS_DIRECTORY),
  language : 'c'
)

lib = shared_library(
  'scroll-extension',
  sources : [webkit_extension],
  dependencies : ext_deps,
  install : true,
  install_dir: WEB_EXTENSIONS_DIRECTORY
)

executable(
  EXECUTABLE_NAME,
  sources : [marker_sources, hoedown_sources],
  dependencies : deps,
  install : true
)

install_data(
  'data/com.github.fabiocolacio.marker.svg',
  install_dir : join_paths(DATA_DIR, 'icons/hicolor/scalable/apps')
)

install_data(
  'data/com.github.fabiocolacio.marker-symbolic.svg',
  install_dir : join_paths(DATA_DIR, 'icons/hicolor/symbolic/apps')
)

install_data(
  'data/com.github.fabiocolacio.marker.desktop',
  install_dir : join_paths(DATA_DIR, 'applications')
)

install_data(
  'data/com.github.fabiocolacio.marker.gschema.xml',
  install_dir : join_paths(DATA_DIR, 'glib-2.0/schemas')
)

install_subdir(
  'data/styles',
  install_dir : APP_DIR
)

install_subdir(
    'data/scripts',
    install_dir : APP_DIR
)

meson.add_install_script('post-install.sh')
