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
	${CMAKE_CURRENT_SOURCE_DIR}/../core
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/nodes/
	${CMAKE_CURRENT_BINARY_DIR}
	${EGO_INCLUDE_DIRS}
	${FILESYSTEM_INCLUDE_DIRS}
	${KAMIKAZE_INCLUDE_DIRS}
	${KANGAO_INCLUDE_DIRS}
	${QT5_CORE_INCLUDE_DIRS}
	${QT5_GUI_INCLUDE_DIRS}
	${QT5_OPENGL_INCLUDE_DIRS}
	${QT5_WIDGETS_INCLUDE_DIRS}
)

set(FICHIERS_KANGAO
	scripts_kangao/menu_debogage.kangao
	scripts_kangao/menu_edition.kangao
	scripts_kangao/menu_fichier.kangao
	scripts_kangao/menu_objet.kangao
	scripts_kangao/menu_prereglage.kangao
)

add_compile_options(-fPIC)

qt5_wrap_cpp(
	test_hdr_moc

	intern/custom_widgets.h
	intern/param_widgets.h

	nodes/node_editorwidget.h

	mainwindow.h
	outliner_widget.h
	properties_widget.h
	timeline_widget.h
	viewer.h
)

add_library(kmk_ui STATIC
	mainwindow.h
	outliner_widget.h
	paramcallback.h
	paramfactory.h
	properties_widget.h
	timeline_widget.h
	utils_ui.h
	viewer.h
	widgetbase.h

	intern/custom_widgets.h
	intern/param_widgets.h

	nodes/node_compound.h
	nodes/node_connection.h
	nodes/node_constants.h
	nodes/node_editorwidget.h
	nodes/node_node.h
	nodes/node_port.h
	nodes/node_scene.h

	${test_hdr_moc}

	mainwindow.cc
	outliner_widget.cc
	paramcallback.cc
	paramfactory.cc
	properties_widget.cc
	timeline_widget.cc
	utils_ui.cc
	viewer.cc
	widgetbase.cc

	intern/custom_widgets.cc
	intern/param_widgets.cc

	nodes/node_compound.cc
	nodes/node_connection.cc
	nodes/node_editorwidget.cc
	nodes/node_node.cc
	nodes/node_port.cc

	${FICHIERS_KANGAO}
)

target_include_directories(kmk_ui PUBLIC "${INC_SYS}")

install(
	FILES ${FICHIERS_KANGAO}
	DESTINATION interface
)
