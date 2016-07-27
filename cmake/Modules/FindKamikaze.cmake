# - Find Kamikaze library
# Find the native KAMIKAZE includes and library
# This module defines
#  KAMIKAZE_INCLUDE_DIRS, where to find kamikaze.h, Set when
#                    KAMIKAZE is found.
#  KAMIKAZE_LIBRARIES, libraries to link against to use KAMIKAZE.
#  KAMIKAZE_ROOT_DIR, The base directory to search for KAMIKAZE.
#                This can also be an environment variable.
#  KAMIKAZE_FOUND, If false, do not try to use KAMIKAZE.
#
# also defined, but not for general use are
#  KAMIKAZE_LIBRARY, where to find the Kamikaze library.

# If KAMIKAZE_ROOT_DIR was defined in the environment, use it.
if(NOT KAMIKAZE_ROOT_DIR AND NOT $ENV{KAMIKAZE_ROOT_DIR} STREQUAL "")
	set(KAMIKAZE_ROOT_DIR $ENV{KAMIKAZE_ROOT_DIR})
endif()

set(_kamikaze_SEARCH_DIRS
	${KAMIKAZE_ROOT_DIR}
	/opt/lib/kamikaze
)

find_path(KAMIKAZE_INCLUDE_DIR
	NAMES
		kamikaze/nodes.h
	HINTS
		${_kamikaze_SEARCH_DIRS}
	PATH_SUFFIXES
		include
)

find_library(KAMIKAZE_LIBRARY
	NAMES
		kamikaze
	HINTS
		${_kamikaze_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set KAMIKAZE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KAMIKAZE DEFAULT_MSG
    KAMIKAZE_LIBRARY KAMIKAZE_INCLUDE_DIR)

if(KAMIKAZE_FOUND)
	set(KAMIKAZE_LIBRARIES ${KAMIKAZE_LIBRARY})
	set(KAMIKAZE_INCLUDE_DIRS ${KAMIKAZE_INCLUDE_DIR} ${KAMIKAZE_INCLUDE_DIR}/../)
else()
	set(KAMIKAZE_FOUND FALSE)
endif()

mark_as_advanced(
	KAMIKAZE_INCLUDE_DIR
	KAMIKAZE_LIBRARY
)

unset(_kamikaze_SEARCH_DIRS)
