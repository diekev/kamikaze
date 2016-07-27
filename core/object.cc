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
#include <kamikaze/context.h>
#include <kamikaze/primitive.h>

#include "graphs/object_graph.h"
#include "graphs/object_nodes.h"

#include "scene.h"
#include "task.h"

#include "ui/paramfactory.h"

Object::Object()
    : Transformable()
    , m_graph(new Graph)
{
	add_prop("Position", property_type::prop_vec3);
	set_prop_default_value_vec3(glm::vec3(0.0f, 0.0f, 0.0f));

	add_prop("Rotation", property_type::prop_vec3);
	set_prop_default_value_vec3(glm::vec3(0.0f, 0.0f, 0.0f));

	add_prop("Size", property_type::prop_vec3);
	set_prop_default_value_vec3(glm::vec3(1.0f, 1.0f, 1.0f));

	recompute_matrix();
}

Object::~Object()
{
	delete m_graph;
	m_cache.clear();
}

PrimitiveCollection *Object::collection() const
{
	return m_collection;
}

void Object::collection(PrimitiveCollection *coll)
{
	m_collection = coll;
}

void Object::addNode(Node *node)
{
	node->setPrimitiveCache(&m_cache);
	m_graph->add(node);
	m_graph->active_node(node);
}

Graph *Object::graph() const
{
	return m_graph;
}

void Object::name(const QString &name)
{
	m_name = name.toStdString();
}

const QString Object::name() const
{
	return QString::fromStdString(m_name);
}

#if 0
void Object::updateMatrix()
{
	const auto m_pos = eval_vec3("Position");
	const auto m_rotation = eval_vec3("Rotation");
	const auto m_scale = eval_vec3("Size");

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_matrix = glm::scale(m_matrix, m_scale);

	m_inv_matrix = glm::inverse(m_matrix);
}
#endif

void Object::clearCache()
{
	m_cache.clear();
}

void Object::addChild(Object *child)
{
	m_children.push_back(child);
	child->parent(this);
}

const std::vector<Object *> &Object::children() const
{
	return m_children;
}

Object *Object::parent() const
{
	return m_parent;
}

void Object::parent(Object *parent)
{
	m_parent = parent;
}

/* ********************************************* */

class GraphEvalTask : public Task {
public:
	GraphEvalTask(const EvaluationContext * const context);

	void start(const EvaluationContext * const context) override;
};

GraphEvalTask::GraphEvalTask(const EvaluationContext * const context)
    : Task(context)
{}

void GraphEvalTask::start(const EvaluationContext * const context)
{
	auto object = context->scene->currentObject();
	auto graph = object->graph();
	auto stack = graph->finished_stack();

	const auto size = static_cast<float>(stack.size());
	auto index = 0;

	m_notifier->signalProgressUpdate(0.0f);

	for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
		Node *node = *iter;

		if (node->inputs().empty()) {
			node->buildCollection(context);
		}
		else {
			node->collection(node->getInputCollection(0ul));
		}

		if (node->collection()) {
			node->process();
		}

		if (!node->outputs().empty()) {
			node->setOutputCollection(0ul, node->collection());
		}

		const float progress = (++index / size) * 100.0f;
		m_notifier->signalProgressUpdate(progress);
	}

	auto output_node = graph->output();
	object->collection(output_node->collection());
}

void eval_graph(const EvaluationContext * const context)
{
	auto scene = context->scene;
	auto ob = scene->currentObject();
	auto graph = ob->graph();
	auto output_node = graph->output();

	if (!output_node->isLinked()) {
		output_node->input(0)->collection = nullptr;
		ob->collection(nullptr);
		return;
	}

	graph->build();

	/* XXX */
	for (Node *node : graph->nodes()) {
		for (OutputSocket *output : node->outputs()) {
			output->collection = nullptr;
		}
	}

	ob->collection(nullptr);
	ob->clearCache();

	GraphEvalTask *t = new(tbb::task::allocate_root()) GraphEvalTask(context);
	tbb::task::enqueue(*t);
}
