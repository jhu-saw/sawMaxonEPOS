# (C) Copyright 2024 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

# Findgclib
#
# Find the Maxon Controller Library (EposCmdLib). 
#
#    EposCmdLib_ROOT            -- root directory for library
#    EposCmdLib_INCLUDE_DIR     -- path to header files
#    EposCmdLib_LIBRARIES       -- path to library files
#    EposCmdLib_FOUND           -- true if package found

cmake_minimum_required(VERSION 3.0)

# Initialize
set(EposCmdLib_FOUND FALSE)
set(EposCmdLib_LIBRARIES "")

# Try to find include directory
find_path(EposCmdLib_INCLUDE_DIR
    NAMES Definitions.h
    HINTS /opt/EposCmdLib_6.8.1.0/include 
          /opt/EposCmdLib/include 
    DOC "Directory for EposCmd library header files"
)

if(EposCmdLib_INCLUDE_DIR)
    # Determine root based on include dir
    get_filename_component(EposCmdLib_ROOT ${EposCmdLib_INCLUDE_DIR} DIRECTORY)

    # Look for the versioned shared object
    find_library(EposCmdLib_LIBRARY
        NAMES EposCmd EposCmd.so.6.8.10
        HINTS ${EposCmdLib_ROOT}/lib/x86_64
              /opt/EposCmdLib_6.8.1.0/lib/x86_64
        DOC "EPOS Command shared library"
    )

    if(EposCmdLib_LIBRARY)
        # Record library and its directory
        set(EposCmdLib_LIBRARIES ${EposCmdLib_LIBRARY})
        get_filename_component(EposCmdLib_LIBRARY_DIR ${EposCmdLib_LIBRARY} DIRECTORY)
        set(EposCmdLib_FOUND TRUE)
    endif()
endif()