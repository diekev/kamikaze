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

#include <QObject>

#include <glm/gtc/matrix_transform.hpp>
#include <kamikaze/primitive.h>

#include "nodes/graph.h"
#include "nodes/nodes.h"

#include "scene.h"
#include "task.h"

#include "ui/paramfactory.h"

Object::Object()
    : Transformable()
    , m_graph(new Graph)
{
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

void Object::setUIParams(ParamCallback *cb)
{
	string_param(cb, "Name", &m_name, "");

	xyz_param(cb, "Position", &m_pos[0], 0.0f, 100.0f);
	xyz_param(cb, "Scale", &m_scale[0], 0.0f, 100.0f);
	xyz_param(cb, "Rotation", &m_rot[0], 0.0f, 360.0f);
}

void Object::clearCache()
{
	m_cache.clear();
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
