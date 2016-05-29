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

#include <glm/gtc/matrix_transform.hpp>
#include <kamikaze/primitive.h>
#include <kamikaze/paramfactory.h>

#include "nodes/graph.h"
#include "nodes/nodes.h"

#include <tbb/task.h>

class GraphEvalTask : public tbb::task {
	Object *m_object;
	Graph *m_graph;

public:
	GraphEvalTask(Object *object, Graph *graph)
	    : m_object(object)
	    , m_graph(graph)
	{}

	tbb::task *execute() override
	{
		m_graph->execute();

		auto output_node = m_graph->output();
		m_object->primitive(output_node->primitive());

		return nullptr;
	}
};

Object::Object()
    : m_graph(new Graph)
{
	updateMatrix();
}

Object::~Object()
{
	delete m_graph;
	m_cache.clear();
}

Primitive *Object::primitive() const
{
	return m_primitive;
}

void Object::primitive(Primitive *prim)
{
	m_primitive = prim;
}

void Object::matrix(const glm::mat4 &m)
{
	m_matrix = m;
}

const glm::mat4 &Object::matrix() const
{
	return m_matrix;
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

void Object::name(const QString &name)
{
	m_name = name;
}

const QString &Object::name() const
{
	return m_name;
}

void Object::setUIParams(ParamCallback *cb)
{
	string_param(cb, "Name", &m_name, "");

	xyz_param(cb, "Position", &m_pos[0], 0.0f, 100.0f);
	xyz_param(cb, "Scale", &m_scale[0], 0.0f, 100.0f);
	xyz_param(cb, "Rotation", &m_rotation[0], 0.0f, 360.0f);
}

void Object::updateMatrix()
{
	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_matrix = glm::scale(m_matrix, m_scale);

	m_inv_matrix = glm::inverse(m_matrix);
}

void Object::clearCache()
{
	m_cache.clear();
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
		primitive = nullptr;
	}

	m_primitives.clear();
}

void eval_graph(Object *ob, bool force)
{
	if (!force) {
		return;
	}

	auto graph = ob->graph();
	auto output_node = graph->output();

	if (!output_node->isLinked()) {
		output_node->input(0)->prim = nullptr;
		ob->primitive(nullptr);
		return;
	}

	graph->build();

	/* XXX */
	for (Node *node : graph->nodes()) {
		for (OutputSocket *output : node->outputs()) {
			output->prim = nullptr;
		}
	}

	ob->primitive(nullptr);
	ob->clearCache();

	GraphEvalTask *t = new(tbb::task::allocate_root()) GraphEvalTask(ob, graph);
	tbb::task::enqueue(*t);
}
