/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "object_ops.h"

#include <kamikaze/context.h>
#include <kamikaze/nodes.h>
#include <kamikaze/primitive.h>

#include "graphs/object_graph.h"
#include "graphs/object_nodes.h"

#include "object.h"
#include "scene.h"

/* *************************** add object command *************************** */

void AddObjectCmd::execute(const Context &context)
{
	m_scene = context.scene;

	m_object = new Object;
	m_object->name(m_name.c_str());

	assert(m_scene != nullptr);
	m_scene->addObject(m_object);
}

void AddObjectCmd::undo()
{
	/* TODO */
}

void AddObjectCmd::redo()
{
	/* TODO */
}

/* **************************** add node command **************************** */

void AddNodeCmd::execute(const Context &context)
{
	m_scene = context.scene;
	auto scene_node = m_scene->active_node();

	if (scene_node == nullptr) {
		return;
	}

	m_object = static_cast<Object *>(scene_node);

	assert(m_object != nullptr);

	auto node = (*context.node_factory)(m_name);
	m_object->addNode(node);

	m_scene->notify_listeners(event_type::node | event_type::added);
}

void AddNodeCmd::undo()
{
	/* TODO */
}

void AddNodeCmd::redo()
{
	/* TODO */
}

/* **************************** add torus command **************************** */

void AddPresetObjectCmd::execute(const Context &context)
{
	m_scene = context.scene;

	if (context.eval_ctx->edit_mode) {
		auto scene_node = m_scene->active_node();

		/* Sanity check. */
		if (scene_node == nullptr) {
			return;
		}

		m_object = static_cast<Object *>(scene_node);
	}
	else {
		m_object = new Object;
		m_object->name(m_name);
	}

	assert(m_object != nullptr);

	auto node = (*context.node_factory)(m_name);
	node->xpos(-300);
	node->ypos(-100);

	m_object->addNode(node);

	auto graph = m_object->graph();
	graph->add_to_selection(node);
	graph->connect(node->output(0), graph->output()->input(0));

	if (!context.eval_ctx->edit_mode) {
		m_scene->addObject(m_object);
		m_scene->evalObjectDag(context, m_object);
	}
	else {
		m_scene->notify_listeners(event_type::node | event_type::added);
	}
}

void AddPresetObjectCmd::undo()
{
	/* TODO */
}

void AddPresetObjectCmd::redo()
{
	/* TODO */
}
