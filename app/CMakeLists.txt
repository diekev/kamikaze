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

set(INC_SYS
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${EGO_INCLUDE_DIRS}
	${FILESYSTEM_INCLUDE_DIRS}
	${KAMIKAZE_INCLUDE_DIRS}
	${QT5_WIDGETS_INCLUDE_DIRS}
)

set(DL_LIBRARIES dl)
set(OPENGL_LIBRARIES GLEW GLU GL glut)
set(TBB_LIBRARIES tbb)
set(FILESYSTEM_LIBS ${FILESYSTEM_LIBRARIES} stdc++fs)

set(LIBS
	${QT5_WIDGETS_LIBRARIES}
	${QT5_GUI_LIBRARIES}
	${QT5_OPENGL_LIBRARIES}
	${QT5_CORE_LIBRARIES}

	kmk_ui
	${QT5_OPENGL_LIBRARIES}
	${OPENGL_LIBRARIES}
	kmk_core
	${EGO_LIBRARIES}
	kmk_util

	${FILESYSTEM_LIBS}
	${DL_LIBRARIES}

	${KAMIKAZE_LIBRARIES}
	${TBB_LIBRARIES}

	${EGO_LIBRARIES}
	${KANGAO_LIBRARIES}
)

add_compile_options(-fPIC)

add_executable(kamikaze main.cc)

target_include_directories(kamikaze PUBLIC "${INC_SYS}")

target_link_libraries(kamikaze "${LIBS}")

install(TARGETS kamikaze RUNTIME DESTINATION .)
