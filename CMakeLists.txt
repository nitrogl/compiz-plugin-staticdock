cmake_minimum_required (VERSION 3.10)
project (compiz-plugin-staticdock CXX)

# This can be overridden on the cmake command line, e.g. for a
# private/local build: -DCOMPIZ_PLUGIN_INSTALL_TYPE=local
if (NOT COMPIZ_PLUGIN_INSTALL_TYPE)
    set (COMPIZ_PLUGIN_INSTALL_TYPE "package" CACHE STRING
         "Where the plugin should be installed (package|compiz|local)")
endif ()

if (NOT CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif ()

if (CMAKE_LIBRARY_ARCHITECTURE AND NOT LIB_SUFFIX)
    set (LIB_SUFFIX "/${CMAKE_LIBRARY_ARCHITECTURE}")
endif ()

find_package (Compiz REQUIRED)
include (CompizPlugin)

compiz_plugin (staticdock PLUGINDEPS composite opengl)

install (
    FILES plugin-staticdock.svg
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ccsm/icons/hicolor/scalable/apps
)
