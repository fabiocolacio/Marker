project('Marker', 'c',
  version : '2017.10.05',
  license : 'GPL3'
)

PREFIX = get_option('prefix')
DATA_DIR = join_paths(PREFIX, 'share')
APP_DIR = join_paths(DATA_DIR, 'com.github.fabiocolacio.marker')
STYLES_DIR = join_paths(APP_DIR, 'styles')
SCRIPTS_DIR = join_paths(APP_DIR, 'scripts')

EXECUTABLE_NAME = 'com.github.fabiocolacio.marker'

gnome = import('gnome')
flex = find_program('flex')
bison = find_program('bison')

pgen = generator(bison,
output : '@BASENAME@.c',
arguments : ['-y', '-v', '-p', 'mtex2MML_yy', '-d', '-o','@OUTPUT@', '@INPUT@'])

pfiles = pgen.process('src/hoedown/mtex2MLL/parser.y')

lgen = generator(flex,
output : '@BASENAME@.c',
arguments : ['-P','mtex2MML_yy', '-o','@OUTPUT@', '@INPUT@'])

lfiles = lgen.process('src/hoedown/mtex2MLL/lexer.l')



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

hoedown_sources = [
  'src/hoedown/autolink.c',
  'src/hoedown/buffer.c',
  'src/hoedown/document.c',
  'src/hoedown/escape.c',
  'src/hoedown/html.c',
  'src/hoedown/html_blocks.c',
  'src/hoedown/html_smartypants.c',
  'src/hoedown/stack.c',
  'src/hoedown/version.c',
  'src/hoedown/mtex2MLL/colors.c',
  'src/hoedown/mtex2MLL/em.c',
  'src/hoedown/mtex2MLL/string_dup.c',
  'src/hoedown/mtex2MLL/environment.c',
  'src/hoedown/mtex2MLL/string_extras.c'
]

deps = [
  dependency('gtk+-3.0'),
  dependency('glib-2.0'),
  dependency('gtksourceview-3.0'),
  dependency('webkit2gtk-4.0')
]

add_global_arguments(
  '-DSTYLES_DIR="@0@/"'.format(STYLES_DIR),
  '-DSCRIPTS_DIR="@0@/"'.format(SCRIPTS_DIR),
  '-DMARKER_VERSION="@0@"'.format(meson.project_version()),
  language : 'c'
)

executable(
  EXECUTABLE_NAME,
  include_directories: include_directories('src/hoedown/mtex2MLL/'),
  sources : [marker_sources, hoedown_sources,  pfiles, lfiles],
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
