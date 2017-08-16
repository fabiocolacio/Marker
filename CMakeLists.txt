cmake_minimum_required(VERSION 2.8)
project("Marker")

set(EXEC_NAME "marker")

find_package(PkgConfig REQUIRED)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)
pkg_search_module(SIGCPP REQUIRED sigc++-2.0)

file(GLOB MARKER_SOURCES src/*.cpp)

add_executable(${EXEC_NAME} ${MARKER_SOURCES})

include_directories(${GTKMM_INCLUDE_DIRS} ${SIGCPP_INCLUDE_DIRS})

set_property(TARGET ${EXEC_NAME}
	     PROPERTY COMPILE_OPTION
	     ${GTKMM_CFLAGS_OTHER}
	     ${SIGCPP_CFLAGS_OTHER})

target_link_libraries(${EXEC_NAME}
	              ${GTKMM_LIBRARIES}
		      ${SIGCPP_LIBRARIES})

install(FILES ${EXEC_NAME}
	RUNTIME DESTINATION bin)
