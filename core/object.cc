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

#include "object.h"

#include <kamikaze/primitive.h>

#include "nodes/graph.h"

Object::Object()
    : m_graph(new Graph)
{}

Object::~Object()
{
	delete m_graph;

	if (m_orig_prim != m_primitive) {
		delete m_orig_prim;
		m_cache.clear();
	}
	else {
		delete m_primitive;
	}
}

Primitive *Object::primitive() const
{
	return m_primitive;
}

void Object::primitive(Primitive *prim)
{
	m_primitive = prim;
	m_orig_prim = prim;
}

void Object::addNode(Node *node)
{
	node->setPrimitiveCache(&m_cache);
	m_graph->add(node);
}

Graph *Object::graph() const
{
	return m_graph;
}

void Object::evalGraph(bool force)
{
	if (!force) {
		return;
	}

	m_graph->build();

	m_cache.clear();
	m_graph->execute();

	auto output_node = m_graph->output();
	m_primitive = output_node->primitive();
}

void Object::name(const QString &name)
{
	m_name = name;
}

const QString &Object::name() const
{
	return m_name;
}

void PrimitiveCache::add(Primitive *prim)
{
	m_primitives.push_back(prim);
}

void PrimitiveCache::clear()
{
	for (auto &primitive : m_primitives) {
		if (primitive->refcount() > 1) {
			primitive->decref();
			continue;
		}

		delete primitive;
	}

	m_primitives.clear();
}
