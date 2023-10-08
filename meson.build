project('Marker', 'c',
  version : '2020.04.04',
  license : 'GPL3'
)

PREFIX = get_option('prefix')
DATA_DIR = join_paths(PREFIX, 'share')
LIBS_DIR = join_paths(PREFIX, 'lib')
APP_DIR = join_paths(DATA_DIR, 'com.github.fabiocolacio.marker')
ICONS_DIR = join_paths(APP_DIR, 'icons')
STYLES_DIR = join_paths(APP_DIR, 'styles')
COMMON_DIR = join_paths(APP_DIR, 'common')
SCRIPTS_DIR = join_paths(APP_DIR, 'scripts')
HIGHLIGHT_STYLES_DIR = join_paths(join_paths(SCRIPTS_DIR, 'highlight'),'styles')
WEB_EXTENSIONS_DIRECTORY = join_paths(LIBS_DIR, 'Marker.extensions')
APPDATA_DIR = join_paths(DATA_DIR, 'metainfo')
LOCALE_DIR = join_paths(PREFIX, get_option('localedir'))

EXECUTABLE_NAME = 'marker'

gnome = import('gnome')

i18n = import('i18n')
subdir('help')
subdir('po')

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
  'src/marker-utils.c',
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
  'src/scidown/src/constants.c',
  'src/scidown/src/charter/src/charter_string.c',
  'src/scidown/src/charter/src/csv_parser/csvparser.c',
  'src/scidown/src/charter/src/tinyexpr/tinyexpr.c',
  'src/scidown/src/charter/src/clist.c',
  'src/scidown/src/charter/src/parser.c',
  'src/scidown/src/charter/src/charter.c',
  'src/scidown/src/charter/src/svg.c',
  'src/scidown/src/charter/src/latex.c',
  'src/scidown/src/charter/src/svg_utils.c'
]

deps = [
  dependency('gtk4', version : '>=4.10'),
  dependency('glib-2.0'),
  dependency('gtksourceview-5'),
  dependency('webkitgtk-6.0'),
  dependency('gtkspell3-3.0')
]

ext_deps =[
  dependency('webkitgtk-6.0')
]

add_global_arguments(
  '-DSTYLES_DIR="@0@/"'.format(STYLES_DIR),
  '-DSCRIPTS_DIR="@0@/"'.format(SCRIPTS_DIR),
  '-DICONS_DIR="@0@/"'.format(ICONS_DIR),
  '-DHIGHLIGHT_STYLES_DIR="@0@/"'.format(HIGHLIGHT_STYLES_DIR),
  '-DMARKER_VERSION="@0@"'.format(meson.project_version()),
  '-DLOCALE_DIR="@0@"'.format(LOCALE_DIR),
  '-DCOMMON_DIR="@0@/"'.format(COMMON_DIR),
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
  'data/common',
  install_dir : APP_DIR
)

install_subdir(
  'data/scripts',
  install_dir : APP_DIR
)

install_subdir(
  'data/icons',
  install_dir : APP_DIR
)

meson.add_install_script('post-install.sh')
