cmake_minimum_required(VERSION 3.1)
project(xfce4-dockbarx-plugin C)

include(GNUInstallDirs)

set(PLUGIN_ID "dockbarx")
set(PLUGIN_VERSION "0.7.1")

set(PLUGIN_LIB_DIR "${CMAKE_INSTALL_LIBDIR}/xfce4/panel/plugins")
set(PLUGIN_DESKTOP_DIR "${CMAKE_INSTALL_DATAROOTDIR}/xfce4/panel/plugins")
set(DOCKBARX_PATH "${CMAKE_INSTALL_FULL_DATAROOTDIR}/dockbarx")

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK REQUIRED gtk+-3.0>=3.12)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

pkg_check_modules(GLIB REQUIRED glib-2.0>=2.38)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

pkg_check_modules(LIBXFCE4_PANEL REQUIRED libxfce4panel-2.0>=4.12)
include_directories(${LIBXFCE4_PANEL_INCLUDE_DIRS})
link_directories(${LIBXFCE4_PANEL_LIBRARY_DIRS})

pkg_check_modules(LIBXFCONF REQUIRED libxfconf-0>=4.12)
include_directories(${LIBXFCONF_INCLUDE_DIRS})
link_directories(${LIBXFCONF_LIBRARY_DIRS})

file(GLOB PLUGIN_SRC "src/*.c" "src/*.h")
add_library(plugin SHARED ${PLUGIN_SRC})
target_link_libraries(plugin ${LIBXFCE4_PANEL_LIBRARIES} ${LIBXFCONF_LIBRARIES} ${GTK_LIBRARIES} ${GLIB_LIBRARIES})
set_target_properties(plugin PROPERTIES OUTPUT_NAME ${PLUGIN_ID})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/xfce4-dockbarx-plug.py DESTINATION ${DOCKBARX_PATH} RENAME xfce4-panel-plug)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/ DESTINATION ${DOCKBARX_PATH}/themes FILES_MATCHING PATTERN "*.tar.gz")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/dockbarx.desktop DESTINATION ${PLUGIN_DESKTOP_DIR})
install(TARGETS plugin LIBRARY DESTINATION ${PLUGIN_LIB_DIR})

