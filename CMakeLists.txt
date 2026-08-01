cmake_minimum_required (VERSION 3.10)
project (compiz-plugin-staticdock CXX)

# Default to a proper system install (matches what a .deb needs) rather
# than compiz_plugin()'s own default of installing into $HOME/.compiz-1.
# This can still be overridden on the cmake command line, e.g. for a
# private/local build: -DCOMPIZ_PLUGIN_INSTALL_TYPE=local
if (NOT COMPIZ_PLUGIN_INSTALL_TYPE)
    set (COMPIZ_PLUGIN_INSTALL_TYPE "package" CACHE STRING
         "Where the plugin should be installed (package|compiz|local)")
endif ()

if (NOT CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif ()

# CompizPlugin.cmake builds its install libdir as
# ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/compiz, and LIB_SUFFIX is
# empty unless we set it - which would install to /usr/lib/compiz
# instead of the Debian multiarch path (/usr/lib/x86_64-linux-gnu/compiz)
# that libcomposite/libopengl and every other real plugin actually live
# in. CMAKE_LIBRARY_ARCHITECTURE is populated automatically by CMake on
# multiarch systems (Debian/Ubuntu), so use that rather than
# hardcoding a triplet.
if (CMAKE_LIBRARY_ARCHITECTURE AND NOT LIB_SUFFIX)
    set (LIB_SUFFIX "/${CMAKE_LIBRARY_ARCHITECTURE}")
endif ()

# Provided by the compiz-dev package: locates compiz via pkg-config and
# adds /usr/share/compiz/cmake (CompizPlugin.cmake, CompizDefaults.cmake,
# etc.) to CMAKE_MODULE_PATH. No need to be nested inside the compiz
# source tree for this to work.
find_package (Compiz REQUIRED)
include (CompizPlugin)

# Only "composite" and "opengl" are genuine compile/link dependencies -
# staticdock.cpp includes their headers (composite/composite.h,
# opengl/opengl.h) and links against libcomposite/libopengl. It never
# includes cube.h or calls CubeScreen::get()/RotateScreen::get()/
# ExpoScreen::get() - the cube/rotate/expo relationship is purely a
# *runtime load-order* requirement (staticdock must be wrapped above
# whichever of them are enabled), which is expressed in
# staticdock.xml.in's <relation type="after"> instead. Listing
# cube/rotate/expo here as PLUGINDEPS would actually break the build,
# since (unlike cube) rotate and expo don't ship a compiz-rotate.pc /
# compiz-expo.pc for other plugins to link against.
compiz_plugin (staticdock PLUGINDEPS composite opengl)
