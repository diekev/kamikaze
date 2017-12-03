# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2016 Kévin Dietrich.
# All rights reserved.
#
# ***** END GPL LICENSE BLOCK *****

set(INCLUSIONS
	${TEST_UNITAIRE_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../
)

set(SOURCES
	main.cc
)

set(BIBLIOTHEQUES_DL dl)
set(OPENGL_LIBRARIES GLEW GLU GL glut)
set(TBB_LIBRARIES tbb)

set(BIBLIOTHEQUES
	${TEST_UNITAIRE_LIBRARIES}

	kmk_core

	${KAMIKAZE_LIBRARIES}
	${QT5_CORE_LIBRARIES}
	${TBB_LIBRARIES}

	${EGO_LIBRARIES}
	${OPENGL_LIBRARIES}
	${BIBLIOTHEQUES_DL}
	${FILESYSTEM_LIBRARIES}
)

add_compile_options(-fPIC)

add_executable(test_kamikaze ${SOURCES})

target_include_directories(test_kamikaze PUBLIC "${INCLUSIONS}")
target_link_libraries(test_kamikaze "${BIBLIOTHEQUES}")

add_test(tests test_kamikaze)
