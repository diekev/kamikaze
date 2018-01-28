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

cmake_minimum_required(VERSION 3.8)

# ------------------------------------------------------------------------------

project(kamikaze_sdk)

# ------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules")

# ------------------------------------------------------------------------------

set(CMAKE_BUILD_TYPE_INIT "Release")

# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-fconcepts)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -Og -Wno-error=unused-function -Wextra -Wno-ignored-qualifiers -Wno-unused-parameter -Wno-unknown-pragmas -Wno-missing-field-initializers -Wno-sign-compare -Wno-type-limits -Wmissing-format-attribute -fno-common -Wundef -Wno-delete-non-virtual-dtor -Wno-div-by-zero  -Wstrict-aliasing=2 -Wsizeof-pointer-memaccess -Wformat=2 -Wno-format-nonliteral -Wno-format-y2k -fstrict-overflow -Wstrict-overflow=2 -Wwrite-strings -Wformat-extra-args -Wlogical-op -Wnonnull -fno-omit-frame-pointer -Wno-error=unused-result -Wno-error=clobbered -fstack-protector-all --param=ssp-buffer-size=4 -Werror -Wno-maybe-uninitialized -Wmissing-include-dirs -Wdate-time -Wuninitialized -Winit-self -Wtype-limits -fno-nonansi-builtins -Warray-bounds -Wno-error=unused-local-typedefs -fdiagnostics-color=always -fsanitize=address -fuse-ld=gold")

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DGLM_FORCE_RADIANS)

# ------------------------------------------------------------------------------

find_package(Ego REQUIRED)
find_package(Kangao REQUIRED)

# ------------------------------------------------------------------------------

set(INC_SYS
	${EGO_INCLUDE_DIRS}
	${KANGAO_INCLUDE_DIRS}
)

set(ENTETES_OUTILS
	outils/chaîne_caractère.h
	outils/géométrie.h
	outils/interpolation.h
	outils/mathématiques.h
	outils/parallélisme.h
	outils/rendu.h
)

set(HEADERS
	attribute.h
	bruit.h
	context.h
	cube.h
	factory.h
	geomlists.h
	mesh.h
	noeud.h
	operateur.h
	prim_points.h
	primitive.h
	renderbuffer.h
	segmentprim.h
)

add_library(kamikaze SHARED
	outils/géométrie.cc

	attribute.cc
	bruit.cc
	context.cc
	cube.cc
	geomlists.cc
	noeud.cc
	mesh.cc
	operateur.cc
	prim_points.cc
	primitive.cc
	renderbuffer.cc
	segmentprim.cc

	${HEADERS}
	${ENTETES_OUTILS}
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

install(
	FILES ${ENTETES_OUTILS}
	DESTINATION include/kamikaze/outils
)
