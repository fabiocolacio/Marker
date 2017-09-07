cmake_minimum_required(VERSION 2.8)
project("Marker")

set(EXEC_NAME "com.github.fabiocolacio.marker")

set(GRES_DIR ${CMAKE_SOURCE_DIR}/gres)
set(STYLES_DIR ${CMAKE_SOURCE_DIR}/styles)
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)
set(CONF_FILE ${DATA_DIR}/marker.config)

set(DATA_DEST ${CMAKE_INSTALL_PREFIX}/share)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lwkhtmltox")

add_definitions(-DRES_DIR="${DATA_DEST}/${EXEC_NAME}")
add_definitions(-DCONF_FILE="${DATA_DEST}/${EXEC_NAME}/marker.config")
add_definitions(-DSTYLES_DIR="${DATA_DEST}/${EXEC_NAME}/styles/")

find_package(PkgConfig REQUIRED)
pkg_search_module(GTK REQUIRED gtk+-3.0)
pkg_search_module(GTKSOURCEVIEW REQUIRED gtksourceview-3.0)
pkg_search_module(WEBKITGTK REQUIRED webkitgtk-3.0)

file(GLOB MARKER_SOURCES src/*.c)

add_executable(${EXEC_NAME} ${MARKER_SOURCES})

include_directories(${GTK_INCLUDE_DIRS}
                    ${GTKSOURCEVIEW_INCLUDE_DIRS}
                    ${WEBKITGTK_INCLUDE_DIRS})

set_property(TARGET ${EXEC_NAME}
             PROPERTY COMPILE_OPTION
             ${GTK_CFLAGS_OTHER}
             ${GTKSOURCEVIEW_CFLAGS_OTHER}
             ${WEBKITGTK_CFLAGS_OTHER})

target_link_libraries(${EXEC_NAME}
                      ${GTK_LIBRARIES}
                      ${GTKSOURCEVIEW_LIBRARIES}
                      ${WEBKITGTK_LIBRARIES})

add_custom_target(gres
                  glib-compile-resources marker.gresource.xml --target="../src/resources.c" --generate-source
                  WORKING_DIRECTORY ${GRES_DIR})
                  
add_dependencies(${EXEC_NAME} gres)

install(TARGETS ${EXEC_NAME}
        RUNTIME DESTINATION bin)

install(FILES ${DATA_DIR}/${EXEC_NAME}.desktop
        DESTINATION ${DATA_DEST}/applications/)

install(FILES ${DATA_DIR}/com.github.fabiocolacio.marker.svg
        DESTINATION ${DATA_DEST}/icons/hicolor/scalable/apps)

install(FILES ${DATA_DIR}/com.github.fabiocolacio.marker-symbolic.svg
        DESTINATION ${DATA_DEST}/icons/hicolor/symbolic/apps)

install(CODE "execute_process(COMMAND gtk-update-icon-cache -f -t ${DATA_DEST}/icons/hicolor)")

install(FILES ${CONF_FILE}
        DESTINATION ${DATA_DEST}/${EXEC_NAME}
        PERMISSIONS WORLD_READ WORLD_WRITE WORLD_EXECUTE)

install(DIRECTORY ${STYLES_DIR}
        DESTINATION ${DATA_DEST}/${EXEC_NAME}
        FILES_MATCHING PATTERN "*"
        PERMISSIONS WORLD_READ WORLD_WRITE WORLD_EXECUTE)
	
