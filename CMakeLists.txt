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

project(kamikaze)

# ------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# ------------------------------------------------------------------------------

set(CMAKE_BUILD_TYPE_INIT "Release")

# ------------------------------------------------------------------------------

add_compile_options(-std=c++14)

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DGLM_FORCE_RADIANS)

# ------------------------------------------------------------------------------

find_package(Ego REQUIRED)
set(EGO_LIBRARIES /opt/lib/ego/lib/libego.a)

find_package(FileSystem REQUIRED)
find_package(Kamikaze REQUIRED)

find_package(Qt5Core REQUIRED)
set(QT5_CORE_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
set(QT5_CORE_LIBRARIES Qt5::Core)

find_package(Qt5Gui REQUIRED)
set(QT5_GUI_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
set(QT5_GUI_LIBRARIES Qt5::Gui)

find_package(Qt5OpenGL REQUIRED)
set(QT5_OPENGL_INCLUDE_DIRS ${Qt5OpenGL_INCLUDE_DIRS})
set(QT5_OPENGL_LIBRARIES Qt5::OpenGL)

find_package(Qt5Widgets REQUIRED)
set(QT5_WIDGETS_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS})
set(QT5_WIDGETS_LIBRARIES Qt5::Widgets)

# ------------------------------------------------------------------------------

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(util)
add_subdirectory(app)
