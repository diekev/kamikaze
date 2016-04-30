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

QT += widgets
QT -= gui

TARGET = kamikaze
VERSION = 0.1
TEMPLATE = lib

CONFIG += shared

include(../global.pri)

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$EGO_INCLUDE_DIR

LIBS += -ltbb

DEFINES += GLM_FORCE_RADIANS

SRC_INTERN = \
    intern/param_widgets.cc \
    intern/custom_widgets.cc

HDR_INTERN = \
    intern/param_widgets.h \
    intern/custom_widgets.h

HDR_SDK = \
    attribute.h \
    context.h \
	cube.h \
    paramfactory.h \
    mesh.h \
    noise.h \
    nodes.h \
    primitive.h \
    geomlists.h \
    util_parallel.h \
    prim_points.h

SOURCES += \
    $$SRC_INTERN \
    context.cc \
	cube.cc \
    paramfactory.cc \
    noise.cc \
    nodes.cc \
    primitive.cc \
    mesh.cc \
    attribute.cc \
    geomlists.cc \
    prim_points.cc

HEADERS += \
    $$HDR_SDK \
    $$HDR_INTERN

unix {
	target.path = /opt/lib/kamikaze/lib
    INSTALLS += target

	header_install.files = $$HDR_SDK
	header_install.path = /opt/lib/kamikaze/include/kamikaze
    INSTALLS += header_install
}
