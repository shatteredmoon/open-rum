# - Locate FMOD library (By Matt Raykowski, OpenNeL Project http://www.opennel.org/)
# http://www.opennel.org/fisheye/browse/~raw,r=1353/NeL/trunk/nel/CMakeModules/FindFMOD.cmake
# (with permission to relicense as LGPL-with-staticlink-exemption by Matt Raykowski)
# This module defines
# FMOD_LIBRARY, the library to link against
# FMOD_FOUND, if false, do not try to link to FMOD
# FMOD_INCLUDE_DIR, where to find headers.

IF(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
# in cache already
SET(FMOD_FIND_QUIETLY TRUE)
ENDIF(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)


FIND_PATH(FMOD_INCLUDE_DIR
          fmod.h
          PATHS
          ${FMOD_DIR}/include
          ${FMOD_DIR}/inc
          /usr/local/include
          /usr/include
          /sw/include
          /opt/local/include
          /opt/csw/include
          /opt/include
          PATH_SUFFIXES fmod fmod3
)

if(MSVC)
    if(CMAKE_CL_64)
        set(FMOD_NAMES ${FMOD_NAMES} fmodex64_vc)
    else()
        set(FMOD_NAMES ${FMOD_NAMES} fmodex_vc)
    endif()
else()
    set(FMOD_NAMES ${FMOD_NAMES} libfmodex)
endif()

FIND_LIBRARY(FMOD_LIBRARY
             NAMES ${FMOD_NAMES}
             PATHS
             ${FMOD_DIR}/lib
             /usr/local/lib
             /usr/lib
             /usr/local/X11R6/lib
             /usr/X11R6/lib
             /sw/lib
             /opt/local/lib
             /opt/csw/lib
             /opt/lib
             /usr/freeware/lib64
)

IF(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
    SET(FMOD_FOUND "YES")
    SET( FMOD_LIBRARIES ${FMOD_LIBRARY} )
    IF(NOT FMOD_FIND_QUIETLY)
        MESSAGE(STATUS "Found FMOD: ${FMOD_LIBRARY}")
    ENDIF(NOT FMOD_FIND_QUIETLY)
ELSE(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
    IF(NOT FMOD_FIND_QUIETLY)
        MESSAGE(STATUS "Warning: Unable to find FMOD!")
    ENDIF(NOT FMOD_FIND_QUIETLY)
ENDIF(FMOD_LIBRARY AND FMOD_INCLUDE_DIR)