# - Find Kangao library
# Find the native KANGAO includes and library
# This module defines
#  KANGAO_INCLUDE_DIRS, where to find kangao.h, Set when
#                    KANGAO is found.
#  KANGAO_LIBRARIES, libraries to link against to use KANGAO.
#  KANGAO_ROOT_DIR, The base directory to search for KANGAO.
#                This can also be an environment variable.
#  KANGAO_FOUND, If false, do not try to use KANGAO.
#
# also defined, but not for general use are
#  KANGAO_LIBRARY, where to find the Kangao library.

# If KANGAO_ROOT_DIR was defined in the environment, use it.
if(NOT KANGAO_ROOT_DIR AND NOT $ENV{KANGAO_ROOT_DIR} STREQUAL "")
	set(KANGAO_ROOT_DIR $ENV{KANGAO_ROOT_DIR})
endif()

set(_kangao_SEARCH_DIRS
	${KANGAO_ROOT_DIR}
	/opt/lib/kangao
)

find_path(KANGAO_INCLUDE_DIR
	NAMES
	    kangao/kangao.h
	HINTS
		${_kangao_SEARCH_DIRS}
	PATH_SUFFIXES
		include
)

find_library(KANGAO_LIBRARY
	NAMES
		kangao
	HINTS
		${_kangao_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set KANGAO_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KANGAO DEFAULT_MSG
    KANGAO_LIBRARY KANGAO_INCLUDE_DIR)

if(KANGAO_FOUND)
	set(KANGAO_LIBRARIES ${KANGAO_LIBRARY})
	set(KANGAO_INCLUDE_DIRS ${KANGAO_INCLUDE_DIR} ${KANGAO_INCLUDE_DIR}/../)
else()
	set(KANGAO_FOUND FALSE)
endif()

mark_as_advanced(
	KANGAO_INCLUDE_DIR
	KANGAO_LIBRARY
)

unset(_kangao_SEARCH_DIRS)
