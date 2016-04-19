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
INCLUDEPATH += /opt/lib/ego/include/

DEFINES += GLM_FORCE_RADIANS

SOURCES += \
    context.cc \
	cube.cc \
    paramfactory.cc \
    param_widgets.cc \
    xyzspinbox.cc \
    noise.cc \
    nodes.cc \
    primitive.cc

HEADERS += \
    context.h \
	cube.h \
    paramfactory.h \
    param_widgets.h \
    xyzspinbox.h \
    noise.h \
    nodes.h \
    primitive.h

unix {
	target.path = /opt/lib/kamikaze/lib
    INSTALLS += target

	header_install.files = $$HEADERS
	header_install.path = /opt/lib/kamikaze/include/kamikaze
    INSTALLS += header_install
}
