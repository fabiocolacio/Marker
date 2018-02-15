project('Marker', 'c',
  version : '2018.01.09',
  license : 'GPL3'
)

PREFIX = get_option('prefix')
DATA_DIR = join_paths(PREFIX, 'share')
APP_DIR = join_paths(DATA_DIR, 'com.github.fabiocolacio.marker')
STYLES_DIR = join_paths(APP_DIR, 'styles')
SCRIPTS_DIR = join_paths(APP_DIR, 'scripts')
HIGHLIGHT_STYLES_DIR = join_paths(join_paths(SCRIPTS_DIR, 'highlight'),'styles')
WEB_EXTENSIONS_DIRECTORY = join_paths(APP_DIR, 'extensions')
APPDATA_DIR = join_paths(DATA_DIR, 'appdata')

EXECUTABLE_NAME = 'marker'

gnome = import('gnome')

res = gnome.compile_resources(
  'marker_resources',
  'src/resources/marker.gresource.xml',
  source_dir : 'src/resources'
)

marker_sources = [
  'src/marker.c',
  'src/marker-editor.c',
  'src/marker-window.c',
  'src/marker-exporter.c',
  'src/marker-markdown.c',
  'src/marker-prefs.c',
  'src/marker-source-view.c',
  'src/marker-preview.c',
  'src/marker-string.c',
  'src/marker-widget.c',
  'src/marker-sketcher-window.c',
  res
]

scroll_extension = [
  'src/webkit-extension/scroll-extension.c'
]

click_extension = [
  'src/webkit-extension/click-extension.c'
]

hoedown_sources = [
  'src/scidown/src/autolink.c',
  'src/scidown/src/buffer.c',
  'src/scidown/src/document.c',
  'src/scidown/src/escape.c',
  'src/scidown/src/html.c',
  'src/scidown/src/utils.c',
  'src/scidown/src/latex.c',
  'src/scidown/src/html_blocks.c',
  'src/scidown/src/html_smartypants.c',
  'src/scidown/src/stack.c',
  'src/scidown/src/version.c',
  'src/scidown/src/charter/csv_parser/csvparser.c',
  'src/scidown/src/charter/tinyexpr/tinyexpr.c',
  'src/scidown/src/charter/clist.c',
  'src/scidown/src/charter/parser.c',
  'src/scidown/src/charter/charter.c',
  'src/scidown/src/charter/renderer.c',
  'src/scidown/src/charter/svg_utils.c'
]

deps = [
  dependency('gtk+-3.0'),
  dependency('glib-2.0'),
  dependency('gtksourceview-3.0'),
  dependency('webkit2gtk-4.0'),
  dependency('gtkspell3-3.0')
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

shared_library(
  'scroll-extension',
  sources : [scroll_extension],
  dependencies : ext_deps,
  install : true,
  link_args: '-lm',
  install_dir: WEB_EXTENSIONS_DIRECTORY
)

executable(
  EXECUTABLE_NAME,
  sources : [marker_sources, hoedown_sources],
  dependencies : deps,
  link_args: '-lm',
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

install_data(
  'data/com.github.fabiocolacio.marker.appdata.xml',
  install_dir : APPDATA_DIR
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
