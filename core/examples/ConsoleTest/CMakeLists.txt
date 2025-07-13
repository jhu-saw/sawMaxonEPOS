#
# (C) Copyright 2024-2025 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.16)
project (sawMaxonConsole VERSION 2.1.0)

# List cisst libraries needed
set (REQUIRED_CISST_LIBRARIES
  cisstCommon
  cisstVector
  cisstOSAbstraction
  cisstMultiTask
  cisstParameterTypes)

# find cisst and make sure the required libraries have been compiled
find_package (cisst 1.2 COMPONENTS ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_set_output_path ()

  find_package (sawMaxonEPOS
    HINTS ${CMAKE_BINARY_DIR})

  if (sawMaxonEPOS_FOUND)

    include_directories (${sawMaxonEPOS_INCLUDE_DIR})
    link_directories (${sawMaxonEPOS_LIBRARY_DIR})

    add_executable (sawMaxonConsole main.cpp)

    # link with the cisst libraries
    cisst_target_link_libraries (sawMaxonConsole ${REQUIRED_CISST_LIBRARIES})

    # link with sawMaxonEPOS library
    target_link_libraries (sawMaxonConsole ${sawMaxonEPOS_LIBRARIES})

    set_target_properties (sawMaxonConsole PROPERTIES
      COMPONENT sawMaxonConsole-Examples
      FOLDER "sawMaxonConsole")

    install (TARGETS sawMaxonConsole
      COMPONENT sawMaxonConsole-Examples
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib)

  else (sawMaxonEPOS_FOUND)
    message ("Information: sawMaxonConsole will not be compiled, it requires sawMaxonEPOS")
  endif (sawMaxonEPOS_FOUND)
else (cisst_FOUND_AS_REQUIRED)
  message ("Information: sawMaxonConsole will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
