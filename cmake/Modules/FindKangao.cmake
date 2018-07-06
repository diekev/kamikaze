# - Find Danjo library
# Find the native DANJO includes and library
# This module defines
#  DANJO_INCLUDE_DIRS, where to find danjo.h, Set when
#                    DANJO is found.
#  DANJO_LIBRARIES, libraries to link against to use DANJO.
#  DANJO_ROOT_DIR, The base directory to search for DANJO.
#                This can also be an environment variable.
#  DANJO_FOUND, If false, do not try to use DANJO.
#
# also defined, but not for general use are
#  DANJO_LIBRARY, where to find the Danjo library.

# If DANJO_ROOT_DIR was defined in the environment, use it.
if(NOT DANJO_ROOT_DIR AND NOT $ENV{DANJO_ROOT_DIR} STREQUAL "")
	set(DANJO_ROOT_DIR $ENV{DANJO_ROOT_DIR})
endif()

set(_danjo_SEARCH_DIRS
	${DANJO_ROOT_DIR}
	/opt/lib/danjo
)

find_path(DANJO_INCLUDE_DIR
	NAMES
	    danjo/danjo.h
	HINTS
		${_danjo_SEARCH_DIRS}
	PATH_SUFFIXES
		include
)

find_library(DANJO_LIBRARY
	NAMES
		danjo
	HINTS
		${_danjo_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set DANJO_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DANJO DEFAULT_MSG
	DANJO_LIBRARY DANJO_INCLUDE_DIR)

if(DANJO_FOUND)
	set(DANJO_LIBRARIES ${DANJO_LIBRARY})
	set(DANJO_INCLUDE_DIRS ${DANJO_INCLUDE_DIR} ${DANJO_INCLUDE_DIR}/../)
else()
	set(DANJO_FOUND FALSE)
endif()

mark_as_advanced(
	DANJO_INCLUDE_DIR
	DANJO_LIBRARY
)

unset(_danjo_SEARCH_DIRS)
