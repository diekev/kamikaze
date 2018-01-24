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
#include <kamikaze/noeud.h>
#include <kamikaze/operateur.h>
#include <kamikaze/primitive.h>

#include "graphs/object_graph.h"
#include "graphs/object_nodes.h"

#include "object.h"
#include "scene.h"

/* *************************** add object command *************************** */

void AddObjectCmd::execute(const Context &context)
{
	m_scene = context.scene;

	m_object = new SceneNode(context);
	m_object->name(m_name.c_str());

	assert(m_scene != nullptr);
	m_scene->add_node(m_object);
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
	auto scene_node = m_scene->current_node();

	if (scene_node == nullptr) {
		return;
	}

	m_object = scene_node;

	assert(m_object != nullptr);

	auto noeud = new Noeud();
	noeud->nom(m_name);

	auto operateur = (*context.usine_operateur)(m_name, noeud, context);
	static_cast<void>(operateur);

	noeud->synchronise_donnees();

	m_object->ajoute_noeud(noeud);

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
	/* TODO: context of creation. */
	auto scene = context.scene;

	m_object = new SceneNode(context);
	m_object->name(m_name);

	auto noeud = new Noeud();
	noeud->posx(-300);
	noeud->posy(-100);

	(*context.usine_operateur)(m_name, noeud, context);

	noeud->synchronise_donnees();

	m_object->ajoute_noeud(noeud);

	auto graph = m_object->graph();
	graph->ajoute_selection(noeud);
	graph->connecte(noeud->sortie(0), graph->sortie()->entree(0));

	scene->add_node(scene_node);
	scene->evalObjectDag(context, scene_node);

	scene->notify_listeners(event_type::object | event_type::added);
}

void AddPresetObjectCmd::undo()
{
	/* TODO */
}

void AddPresetObjectCmd::redo()
{
	/* TODO */
}
