#
# (C) Copyright 2025 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required(VERSION 3.16)
project (sawMaxonEPOSCore VERSION 0.1.0)

# to locate cisst macros
find_package (cisst REQUIRED cisstCommon)
include (${CISST_USE_FILE})

cisst_cpack_settings (
  VENDOR "JHU"
  MAINTAINER "hwei15@jhu.edu")

add_subdirectory (components)

set (sawMaxonEPOS_DIR "${sawMaxonEPOSCore_BINARY_DIR}/components")
add_subdirectory (examples)
add_subdirectory (share)

include (CPack)
cpack_add_component (sawMaxonEPOS)
cpack_add_component (sawMaxonEPOS-dev
                     DEPENDS sawMaxonEPOS)
cpack_add_component (sawMaxonEPOS-Examples
                     DEPENDS sawMaxonEPOS)
cpack_add_component (sawMaxonEPOS-Share
                     DEPENDS sawMaxonEPOS sawMaxonEPOS-Share)
