#!/usr/bin/env bash

make MAKEFILE_CONF=msys2-Makefile.conf \
     DEPENDENCIES_CHECK=off \
     DOWNLOAD_GAMEPACKS=no \
     INSTALL_DLLS=no \
     BUILD=debug \
     RADIANT_ABOUTMSG="NetRadiant custom dev build" \
     CFLAGS="-g3 -O0" \
     CXXFLAGS="-g3 -O0" \
     LDFLAGS="-g3" \
     -j$(nproc)