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
# The Original Code is Copyright (C) 2015 Kévin Dietrich.
# All rights reserved.
#
# ***** END GPL LICENSE BLOCK *****

CONFIG += no_keywords

include(../../repos/seppuku/rcfiles/build_flags.pri)

QMAKE_CXXFLAGS -= -Weffc++

KAMIKAZE_SDK_INCLUDE_DIR = /opt/lib/kamikaze/include
KAMIKAZE_SDK_LIBRARY = -L/opt/lib/kamikaze/lib -lkamikaze

EGO_INCLUDE_DIR = /opt/lib/ego/include
EGO_LIBRARY = /opt/lib/ego/lib/libego.a
EGO_LIBRARIES = $$EGO_LIBRARY -lGL -lGLEW

FILESYSTEM_INCLUDE_DIR = /opt/lib/utils/include
FILESYSTEM_LIBRARY = /opt/lib/utils/lib/libfilesystem.a
FILESYSTEM_LIBRARIES = $$FILESYSTEM_LIBRARY -ldl
