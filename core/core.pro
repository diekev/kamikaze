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

QT += core
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = core
TEMPLATE = lib

CONFIG += staticlib
CONFIG += no_keywords

include(../global.pri)

INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../util
INCLUDEPATH += $$EGO_INCLUDE_DIR
INCLUDEPATH += $$KAMIKAZE_SDK_INCLUDE_DIR
INCLUDEPATH += $$FILESYSTEM_INCLUDE_DIR

DEFINES += GLM_FORCE_RADIANS

SOURCES += \
    grid.cc \
    object_ops.cc \
    undo.cc \
    camera.cc \
    scene.cc \
#    brush.cc \
    kamikaze_main.cc \
    object.cc \
    graphs/graph_dumper.cc \
    task.cc \
    graphs/depsgraph.cc \
    graphs/object_graph.cc \
    graphs/object_nodes.cc \
    manipulator.cc \
    transformable.cc \
    context.cc

HEADERS += \
    grid.h \
    object_ops.h \
    undo.h \
    factory.h \
    camera.h \
    scene.h \
#    brush.h \
    kamikaze_main.h \
    object.h \
    graphs/graph_dumper.h \
    task.h \
    graphs/depsgraph.h \
    graphs/object_nodes.h \
    graphs/object_graph.h \
    manipulator.h \
    transformable.h \
    context.h

OTHER_FILES += \
    shaders/flat_shader.frag \
    shaders/flat_shader.vert \
    shaders/object.frag \
    shaders/object.vert \
    shaders/volume.frag \
    shaders/volume.vert \
    shaders/tree_topology.frag \
    shaders/tree_topology.vert
