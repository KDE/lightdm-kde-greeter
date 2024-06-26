# - Try to find libxcb
# Once done this will define
#
#  LIBXCB_FOUND - system has libxcb
#  LIBXCB_LIBRARIES - Link these to use libxcb
#  LIBXCB_INCLUDE_DIR - the libxcb include dir
#  LIBXCB_DEFINITIONS - compiler switches required for using libxcb

# Copyright (c) 2008 Helio Chissini de Castro, <helio@kde.org>
# Copyright (c) 2007, Matthias Kretz, <kretz@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause


IF (NOT WIN32)
  IF (LIBXCB_INCLUDE_DIR AND LIBXCB_LIBRARIES)
    # in cache already
    SET(XCB_FIND_QUIETLY TRUE)
  ENDIF (LIBXCB_INCLUDE_DIR AND LIBXCB_LIBRARIES)


  FIND_PACKAGE(PkgConfig)
  # PKG_CHECK_MODULES(PKG_XCB xcb)

  SET(LIBXCB_DEFINITIONS ${PKG_XCB_CFLAGS})

  FIND_PATH(LIBXCB_INCLUDE_DIR xcb/xcb.h ${PKG_XCB_INCLUDE_DIRS})
  FIND_LIBRARY(LIBXCB_LIBRARIES NAMES xcb libxcb PATHS ${PKG_XCB_LIBRARY_DIRS})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCB DEFAULT_MSG LIBXCB_LIBRARIES LIBXCB_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBXCB_INCLUDE_DIR LIBXCB_LIBRARIES XCBPROC_EXECUTABLE)
ENDIF (NOT WIN32)
