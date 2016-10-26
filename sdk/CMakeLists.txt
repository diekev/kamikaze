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

# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

# ------------------------------------------------------------------------------

project(kamikaze_sdk)

# ------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules")

# ------------------------------------------------------------------------------

set(CMAKE_BUILD_TYPE_INIT "Release")

# ------------------------------------------------------------------------------

add_compile_options(-std=c++14)
add_compile_options(-fuse-ld=gold)

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DGLM_FORCE_RADIANS)

# ------------------------------------------------------------------------------

find_package(Ego REQUIRED)
set(EGO_LIBRARIES /opt/lib/ego/lib/libego.a)

# ------------------------------------------------------------------------------

set(INC_SYS
	${EGO_INCLUDE_DIRS}
)

set(HEADERS
	any.h
	attribute.h
	context.h
	cube.h
	factory.h
	geomlists.h
	mesh.h
	nodes.h
	noise.h
	persona.h
	prim_points.h
	primitive.h
	renderbuffer.h
	utils_glm.h
	util_parallel.h
	util_render.h
	util_string.h
)

add_library(kamikaze SHARED
	any.cc 
	attribute.cc
	context.cc
	cube.cc
	geomlists.cc
	nodes.cc
	noise.cc
	mesh.cc
	persona.cc
	prim_points.cc
	primitive.cc
	renderbuffer.cc

	${HEADERS}
)

target_include_directories(kamikaze PUBLIC "${INC_SYS}")

set(KAMIKAZE_VERSION_MAJOR 0)
set(KAMIKAZE_VERSION_MINOR 1)
set(KAMIKAZE_VERSION_PATCH 0)
set(KAMIKAZE_VERSION_STRING ${KAMIKAZE_VERSION_MAJOR}.${KAMIKAZE_VERSION_MINOR}.${KAMIKAZE_VERSION_PATCH})

set_target_properties(kamikaze PROPERTIES VERSION ${KAMIKAZE_VERSION_STRING} SOVERSION ${KAMIKAZE_VERSION_MAJOR})

install(TARGETS kamikaze LIBRARY DESTINATION lib)

install(
	FILES ${HEADERS}
	DESTINATION include/kamikaze
)
