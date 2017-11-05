# - Find FileSystem library
# Find the native FILESYSTEM includes and library
# This module defines
#  FILESYSTEM_INCLUDE_DIRS, where to find filesystem.h, Set when
#                    FILESYSTEM is found.
#  FILESYSTEM_LIBRARIES, libraries to link against to use FILESYSTEM.
#  FILESYSTEM_ROOT_DIR, The base directory to search for FILESYSTEM.
#                This can also be an environment variable.
#  FILESYSTEM_FOUND, If false, do not try to use FILESYSTEM.
#
# also defined, but not for general use are
#  FILESYSTEM_LIBRARY, where to find the FileSystem library.

# If FILESYSTEM_ROOT_DIR was defined in the environment, use it.
if(NOT FILESYSTEM_ROOT_DIR AND NOT $ENV{FILESYSTEM_ROOT_DIR} STREQUAL "")
	set(FILESYSTEM_ROOT_DIR $ENV{FILESYSTEM_ROOT_DIR})
endif()

set(_filesystem_SEARCH_DIRS
	${FILESYSTEM_ROOT_DIR}
	/opt/lib/girafeenfeu
)

find_path(FILESYSTEM_INCLUDE_DIR
	NAMES
		systeme_fichier/utilitaires.h
	HINTS
		${_filesystem_SEARCH_DIRS}
	PATH_SUFFIXES
	    include/girafeenfeu
)

find_library(FILESYSTEM_LIBRARY
	NAMES
		systeme_fichier
	HINTS
		${_filesystem_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set FILESYSTEM_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FILESYSTEM DEFAULT_MSG
    FILESYSTEM_LIBRARY FILESYSTEM_INCLUDE_DIR)

if(FILESYSTEM_FOUND)
	set(FILESYSTEM_LIBRARIES ${FILESYSTEM_LIBRARY})
	set(FILESYSTEM_INCLUDE_DIRS ${FILESYSTEM_INCLUDE_DIR} ${FILESYSTEM_INCLUDE_DIR}/../)
else()
	set(FILESYSTEM_FOUND FALSE)
endif()

mark_as_advanced(
	FILESYSTEM_INCLUDE_DIR
	FILESYSTEM_LIBRARY
)

unset(_filesystem_SEARCH_DIRS)
