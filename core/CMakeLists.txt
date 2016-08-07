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
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${EGO_INCLUDE_DIRS}
	${FILESYSTEM_INCLUDE_DIRS}
	${KAMIKAZE_INCLUDE_DIRS}
	${QT5_CORE_INCLUDE_DIRS}
	${QT5_WIDGETS_INCLUDE_DIRS}
)

add_compile_options(-fPIC)

qt5_wrap_cpp(
	task_notifier_moc

	task.h
)

set(SHADERS
	shaders/flat_shader.frag
	shaders/flat_shader.vert
	shaders/object.frag
	shaders/object.vert
	shaders/tree_topology.frag
	shaders/tree_topology.vert
	shaders/volume.frag
	shaders/volume.vert
)

add_library(kmk_core STATIC
	camera.h
	context.h
	grid.h
	kamikaze_main.h
	object.h
	object_ops.h
	scene.h
	simulation.h
	task.h
	undo.h

	${task_notifier_moc}

	graphs/depsgraph.h
	graphs/graph_dumper.h
	graphs/graph_tools.h
	graphs/object_graph.h
	graphs/object_nodes.h

	camera.cc
	context.cc
	grid.cc
	kamikaze_main.cc
	object.cc
	object_ops.cc
	scene.cc
	simulation.cc
	task.cc
	undo.cc

	graphs/depsgraph.cc
	graphs/graph_dumper.cc
	graphs/object_graph.cc
	graphs/object_nodes.cc

	${SHADERS}
)

target_include_directories(kmk_core PUBLIC "${INC_SYS}")

install(
	FILES ${SHADERS}
	DESTINATION shaders
)

set(ICONS
	icons/icon_box.png
	icons/icon_circle.png
	icons/icon_cone.png
	icons/icon_grid.png
	icons/icon_icosphere.png
	icons/icon_jump_first.png
	icons/icon_jump_last.png
	icons/icon_pause.png
	icons/icon_play_forward.png
	icons/icon_play_backward.png
	icons/icon_point_cloud_cube.png
	icons/icon_step_forward.png
	icons/icon_step_backward.png
	icons/icon_stop.png
	icons/icon_torus.png
	icons/icon_tube.png
	icons/icon_uvsphere.png
)

install(
	FILES ${ICONS}
	DESTINATION icons
)
