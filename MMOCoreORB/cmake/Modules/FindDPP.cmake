# CMake module to find Discord++ (DPP) library

# This module defines:
# DPP_FOUND - True if DPP is found
# DPP_INCLUDE_DIR - DPP's include directory
# DPP_INCLUDE_DIRS - DPP's include directories (same as DPP_INCLUDE_DIR)
# DPP_LIBRARIES - Libraries needed to use DPP
# DPP_VERSION - Version of DPP found

IF (DPP_INCLUDE_DIR)
    SET(DPP_FIND_QUIETLY_INCLUDE TRUE)
ENDIF (DPP_INCLUDE_DIR)

IF (DPP_LIBRARIES)
    SET(DPP_FIND_QUIETLY_LIB TRUE)
ENDIF (DPP_LIBRARIES)

# First try to use pkg-config if available
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_DPP QUIET dpp)
endif()

# Find the include directory
FIND_PATH(DPP_INCLUDE_DIR dpp/dpp.h
    HINTS ${PC_DPP_INCLUDEDIR} ${PC_DPP_INCLUDE_DIRS}
    PATHS
        /usr/local/include
        /usr/include
        /opt/local/include
        ${CMAKE_PREFIX_PATH}/include
    PATH_SUFFIXES dpp
    NO_DEFAULT_PATH
)

# Also check without the dpp subdirectory
if(NOT DPP_INCLUDE_DIR)
    FIND_PATH(DPP_INCLUDE_DIR dpp/dpp.h
        HINTS ${PC_DPP_INCLUDEDIR} ${PC_DPP_INCLUDE_DIRS}
        PATHS
            /usr/local/include
            /usr/include
            /opt/local/include
            ${CMAKE_PREFIX_PATH}/include
    )
endif()

# Find the library
FIND_LIBRARY(DPP_LIBRARIES
    NAMES dpp
    HINTS ${PC_DPP_LIBDIR} ${PC_DPP_LIBRARY_DIRS}
    PATHS
        /usr/local/lib
        /usr/lib
        /opt/local/lib
        ${CMAKE_PREFIX_PATH}/lib
    PATH_SUFFIXES 
        x86_64-linux-gnu
        lib64
)

# Try to get version from pkg-config first
if(PC_DPP_VERSION)
    set(DPP_VERSION ${PC_DPP_VERSION})
elseif(DPP_INCLUDE_DIR AND EXISTS "${DPP_INCLUDE_DIR}/dpp/version.h")
    # Try to extract version from header file
    file(STRINGS "${DPP_INCLUDE_DIR}/dpp/version.h" DPP_VERSION_MAJOR_LINE REGEX "^#define[ \t]+DPP_VERSION_MAJOR[ \t]+[0-9]+.*$")
    file(STRINGS "${DPP_INCLUDE_DIR}/dpp/version.h" DPP_VERSION_MINOR_LINE REGEX "^#define[ \t]+DPP_VERSION_MINOR[ \t]+[0-9]+.*$")
    file(STRINGS "${DPP_INCLUDE_DIR}/dpp/version.h" DPP_VERSION_PATCH_LINE REGEX "^#define[ \t]+DPP_VERSION_PATCH[ \t]+[0-9]+.*$")
    
    if(DPP_VERSION_MAJOR_LINE AND DPP_VERSION_MINOR_LINE AND DPP_VERSION_PATCH_LINE)
        string(REGEX REPLACE "^#define[ \t]+DPP_VERSION_MAJOR[ \t]+([0-9]+).*$" "\\1" DPP_VERSION_MAJOR "${DPP_VERSION_MAJOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+DPP_VERSION_MINOR[ \t]+([0-9]+).*$" "\\1" DPP_VERSION_MINOR "${DPP_VERSION_MINOR_LINE}")
        string(REGEX REPLACE "^#define[ \t]+DPP_VERSION_PATCH[ \t]+([0-9]+).*$" "\\1" DPP_VERSION_PATCH "${DPP_VERSION_PATCH_LINE}")
        set(DPP_VERSION "${DPP_VERSION_MAJOR}.${DPP_VERSION_MINOR}.${DPP_VERSION_PATCH}")
    endif()
endif()

# Set DPP_INCLUDE_DIRS for compatibility
set(DPP_INCLUDE_DIRS ${DPP_INCLUDE_DIR})

# Check if we found everything
IF (DPP_INCLUDE_DIR AND DPP_LIBRARIES)
    SET(DPP_FOUND TRUE)
ELSE ()
    SET(DPP_FOUND FALSE)
    SET(DPP_LIBRARIES)
    SET(DPP_INCLUDE_DIR)
    SET(DPP_INCLUDE_DIRS)
ENDIF ()

# Handle the find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DPP
    REQUIRED_VARS DPP_LIBRARIES DPP_INCLUDE_DIR
    VERSION_VAR DPP_VERSION
)

IF (DPP_FOUND)
    IF (NOT DPP_FIND_QUIETLY_INCLUDE)
        if(DPP_VERSION)
            MESSAGE(STATUS "Found DPP version: ${DPP_VERSION}")
        endif()
        MESSAGE(STATUS "Found DPP includes: ${DPP_INCLUDE_DIR}")
    ENDIF (NOT DPP_FIND_QUIETLY_INCLUDE)

    IF (NOT DPP_FIND_QUIETLY_LIB)
        MESSAGE(STATUS "Found DPP library: ${DPP_LIBRARIES}")
    ENDIF (NOT DPP_FIND_QUIETLY_LIB)
ELSE (DPP_FOUND)
    IF (DPP_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could NOT find DPP library")
    ENDIF (DPP_FIND_REQUIRED)
ENDIF (DPP_FOUND)

MARK_AS_ADVANCED(
    DPP_LIBRARIES
    DPP_INCLUDE_DIR
    DPP_INCLUDE_DIRS
) 