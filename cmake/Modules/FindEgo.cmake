# - Find Ego library
# Find the native EGO includes and library
# This module defines
#  EGO_INCLUDE_DIRS, where to find ego.h, Set when
#                    EGO is found.
#  EGO_LIBRARIES, libraries to link against to use EGO.
#  EGO_ROOT_DIR, The base directory to search for EGO.
#                This can also be an environment variable.
#  EGO_FOUND, If false, do not try to use EGO.
#
# also defined, but not for general use are
#  EGO_LIBRARY, where to find the EGO library.

# If EGO_ROOT_DIR was defined in the environment, use it.
if(NOT EGO_ROOT_DIR AND NOT $ENV{EGO_ROOT_DIR} STREQUAL "")
	set(EGO_ROOT_DIR $ENV{EGO_ROOT_DIR})
endif()

set(_ego_SEARCH_DIRS
	${EGO_ROOT_DIR}
	/opt/lib/ego
)

find_path(EGO_INCLUDE_DIR
	NAMES
		ego/bufferobject.h
	HINTS
		${_ego_SEARCH_DIRS}
	PATH_SUFFIXES
		include
)

find_library(EGO_LIBRARY
	NAMES
		ego
	HINTS
		${_ego_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set EGO_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EGO DEFAULT_MSG
		EGO_LIBRARY EGO_INCLUDE_DIR)

if(EGO_FOUND)
	set(EGO_LIBRARIES ${EGO_LIBRARY})
	set(EGO_INCLUDE_DIRS ${EGO_INCLUDE_DIR})
else()
	set(EGO_EGO_FOUND FALSE)
endif()

mark_as_advanced(
	EGO_INCLUDE_DIR
	EGO_LIBRARY
)

unset(_ego_SEARCH_DIRS)
