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

QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ui
TEMPLATE = lib

CONFIG += staticlib

include(../global.pri)

DEFINES += GLM_FORCE_RADIANS

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../core
INCLUDEPATH += $$PWD/../util
INCLUDEPATH += nodes/
INCLUDEPATH += $$EGO_INCLUDE_DIR
INCLUDEPATH += $$KAMIKAZE_SDK_INCLUDE_DIR

SOURCES += \
    mainwindow.cc \
    nodes/node_compound.cc \
    nodes/node_connection.cc \
    nodes/node_editorwidget.cc \
    nodes/node_node.cc \
    nodes/node_port.cc \
    nodes/node_porttype.cc \
    paramcallback.cc \
    utils_ui.cc

HEADERS += \
    mainwindow.h \
    nodes/node_compound.h \
    nodes/node_connection.h \
    nodes/node_constants.h \
    nodes/node_editorwidget.h \
    nodes/node_node.h \
    nodes/node_port.h \
    nodes/node_porttype.h \
    nodes/node_scene.h \
    paramcallback.h \
    utils_ui.h

FORMS += \
    mainwindow.ui
