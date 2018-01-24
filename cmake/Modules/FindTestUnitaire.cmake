# - Find Kamikaze library
# Find the native TEST_UNITAIRE includes and library
# This module defines
#  TEST_UNITAIRE_INCLUDE_DIRS, where to find test_unitaire.h, Set when
#                    TEST_UNITAIRE is found.
#  TEST_UNITAIRE_LIBRARIES, libraries to link against to use TEST_UNITAIRE.
#  TEST_UNITAIRE_ROOT_DIR, The base directory to search for TEST_UNITAIRE.
#                This can also be an environment variable.
#  TEST_UNITAIRE_FOUND, If false, do not try to use TEST_UNITAIRE.
#
# also defined, but not for general use are
#  TEST_UNITAIRE_LIBRARY, where to find the Kamikaze library.

# If TEST_UNITAIRE_ROOT_DIR was defined in the environment, use it.
if(NOT TEST_UNITAIRE_ROOT_DIR AND NOT $ENV{TEST_UNITAIRE_ROOT_DIR} STREQUAL "")
	set(TEST_UNITAIRE_ROOT_DIR $ENV{TEST_UNITAIRE_ROOT_DIR})
endif()

set(_test_unitaire_SEARCH_DIRS
	${TEST_UNITAIRE_ROOT_DIR}
	/opt/lib/numero7
)

find_path(TEST_UNITAIRE_INCLUDE_DIR
	NAMES
	    test_unitaire/test_unitaire.h
	HINTS
		${_test_unitaire_SEARCH_DIRS}
	PATH_SUFFIXES
		include/numero7
)

find_library(TEST_UNITAIRE_LIBRARY
	NAMES
		test_unitaire
	HINTS
		${_test_unitaire_SEARCH_DIRS}
	PATH_SUFFIXES
		lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set TEST_UNITAIRE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TEST_UNITAIRE DEFAULT_MSG
    TEST_UNITAIRE_LIBRARY TEST_UNITAIRE_INCLUDE_DIR)

if(TEST_UNITAIRE_FOUND)
	set(TEST_UNITAIRE_LIBRARIES ${TEST_UNITAIRE_LIBRARY})
	set(TEST_UNITAIRE_INCLUDE_DIRS ${TEST_UNITAIRE_INCLUDE_DIR} ${TEST_UNITAIRE_INCLUDE_DIR}/../)
else()
	set(TEST_UNITAIRE_FOUND FALSE)
endif()

mark_as_advanced(
	TEST_UNITAIRE_INCLUDE_DIR
	TEST_UNITAIRE_LIBRARY
)

unset(_test_unitaire_SEARCH_DIRS)
