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

#include "depsgraph.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <kamikaze/context.h>
#include <kamikaze/nodes.h>

#include "graph_dumper.h"
#include "graph_tools.h"

#include "object.h"
#include "object_graph.h"
#include "object_nodes.h"
#include "scene.h"
#include "task.h"

//#define DEBUG_DEPSGRAPH

/* ************************************************************************** */

DepsNode::DepsNode()
{
	m_input.parent = this;
	m_output.parent = this;
}

DepsInputSocket *DepsNode::input()
{
	return &m_input;
}

const DepsInputSocket *DepsNode::input() const
{
	return &m_input;
}

DepsOutputSocket *DepsNode::output()
{
	return &m_output;
}

bool DepsNode::is_linked() const
{
	return (m_input.link != nullptr) || (!m_output.links.empty());
}

/* ************************************************************************** */

DepsObjectNode::DepsObjectNode(Object *object)
    : m_object(object)
{}

void DepsObjectNode::pre_process()
{
	/* TODO: what's the purpose of this again? */
	m_object->collection(nullptr);
	m_object->clearCache();
}

void DepsObjectNode::process(const EvaluationContext * const /*context*/, TaskNotifier */*notifier*/)
{
	/* The graph should already have been updated. */
	auto graph = m_object->graph();
	auto output_node = graph->output();
	m_object->collection(output_node->collection());
}

Object *DepsObjectNode::object()
{
	return m_object;
}

const Object *DepsObjectNode::object() const
{
	return m_object;
}

const char *DepsObjectNode::name() const
{
	return m_object->name().toStdString().c_str();
}

/* ************************************************************************** */

ObjectGraphDepsNode::ObjectGraphDepsNode(Graph *graph)
    : m_graph(graph)
{}

void ObjectGraphDepsNode::process(const EvaluationContext * const context, TaskNotifier *notifier)
{
	auto output_node = m_graph->output();

	if (!output_node->isLinked()) {
		output_node->input(0)->collection = nullptr;
		return;
	}

	m_graph->build();

	/* XXX */
	for (Node *node : m_graph->nodes()) {
		for (OutputSocket *output : node->outputs()) {
			output->collection = nullptr;
		}
	}

	auto stack = m_graph->finished_stack();

	const auto size = static_cast<float>(stack.size());
	auto index = 0;

	if (notifier) {
		notifier->signalProgressUpdate(0.0f);
	}

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

		if (notifier) {
			const float progress = (++index / size) * 100.0f;
			notifier->signalProgressUpdate(progress);
		}
	}
}

Graph *ObjectGraphDepsNode::graph()
{
	return m_graph;
}

const Graph *ObjectGraphDepsNode::graph() const
{
	return m_graph;
}

const char *ObjectGraphDepsNode::name() const
{
	return "Object Graph";
}

/* ************************************************************************** */

void TimeDepsNode::process(const EvaluationContext * const /*context*/, TaskNotifier */*notifier*/)
{
	/* Pass. */
}

const char *TimeDepsNode::name() const
{
	return "Scene Time";
}

/* ************************************************************************** */

/* Evaluate depsgraph in another thread. */

class GraphEvalTask : public Task {
	Depsgraph *m_graph;
	DepsNode *m_root;

public:
	GraphEvalTask(Depsgraph *graph, const EvaluationContext * const context, DepsNode *root);

	void start(const EvaluationContext * const context) override;
};

GraphEvalTask::GraphEvalTask(Depsgraph *graph, const EvaluationContext * const context, DepsNode *root)
    : Task(context)
    , m_graph(graph)
    , m_root(root)
{}

void GraphEvalTask::start(const EvaluationContext * const context)
{
	m_graph->evaluate_ex(context, m_root, m_notifier.get());
}

/* ************************************************************************** */

Depsgraph::Depsgraph()
    : m_time_node(new TimeDepsNode)
{
	m_nodes.push_back(m_time_node);
}

Depsgraph::~Depsgraph()
{
	for (auto &node : m_nodes) {
		delete node;
	}
}

void Depsgraph::connect(DepsOutputSocket *from, DepsInputSocket *to)
{
	if (to->link != nullptr) {
		std::cerr << "Input already connected!\n";
		return;
	}

	to->link = from;
	from->links.push_back(to);

	m_need_update = true;
}

void Depsgraph::disconnect(DepsOutputSocket *from, DepsInputSocket *to)
{
	auto iter = std::find(from->links.begin(), from->links.end(), to);

	if (iter == from->links.end()) {
		std::cerr << "Connection mismatch!\n";
		return;
	}

	from->links.erase(iter);
	to->link = nullptr;

	m_need_update = true;
}

void Depsgraph::create_node(Object *object)
{
	DepsObjectNode *node = new DepsObjectNode(object);

	m_nodes.push_back(node);
	m_object_map[object] = node;

	ObjectGraphDepsNode *graph_node = new ObjectGraphDepsNode(object->graph());

	m_nodes.push_back(graph_node);
	m_object_graph_map[object->graph()] = graph_node;

	/* Object depends on its graph. */
	connect(graph_node->output(), node->input());

	m_need_update = true;
}

void Depsgraph::remove_node(Object *object)
{
	/* First, remove graph node. */
	{
		auto iter = m_object_graph_map.find(object->graph());
		assert(iter != m_object_graph_map.end());

		DepsNode *node = iter->second;

		auto node_iter = std::find(m_nodes.begin(), m_nodes.end(), node);
		assert(node_iter != m_nodes.end());

		/* Disconnect input. */
		if (node->input()->link) {
			disconnect(node->input()->link, node->input());
		}

		/* Disconnect output. */
		for (DepsInputSocket *input : node->output()->links) {
			disconnect(node->output(), input);
		}

		m_object_graph_map.erase(iter);
		m_nodes.erase(node_iter);
		delete node;
	}

	/* Then, delete object node. */
	{
		auto iter = m_object_map.find(object);
		assert(iter != m_object_map.end());

		DepsNode *node = iter->second;

		auto node_iter = std::find(m_nodes.begin(), m_nodes.end(), node);
		assert(node_iter != m_nodes.end());

		/* Disconnect output. */
		for (DepsInputSocket *input : node->output()->links) {
			disconnect(node->output(), input);
		}

		m_object_map.erase(iter);
		m_nodes.erase(node_iter);
		delete node;
	}

	m_need_update = true;
}

void Depsgraph::connect_to_time(Object *object)
{
	auto iter = m_object_graph_map.find(object->graph());
	assert(iter != m_object_graph_map.end());

	DepsNode *node = iter->second;
	connect(m_time_node->output(), node->input());
}

void Depsgraph::evaluate(const EvaluationContext * const context, Object *object)
{
	auto iter = m_object_graph_map.find(object->graph());
	assert(iter != m_object_graph_map.end());

	DepsNode *node = iter->second;

	m_need_update = (m_state != DEG_STATE_OBJECT);
	m_state = DEG_STATE_OBJECT;

	/* XXX - see comment in evaluate_ex */
	{
		if (m_need_update) {
			build(node);
			m_need_update = false;
		}

		for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
			DepsNode *node = *iter;
			node->pre_process();
		}
	}

	GraphEvalTask *t = new(tbb::task::allocate_root()) GraphEvalTask(this, context, node);
	tbb::task::enqueue(*t);
}

void Depsgraph::evaluate_for_time_change(const EvaluationContext * const context)
{
	m_need_update = (m_state != DEG_STATE_TIME);
	m_state = DEG_STATE_TIME;

	evaluate_ex(context, m_time_node, nullptr);
}

void Depsgraph::evaluate_ex(const EvaluationContext* const context, DepsNode *root, TaskNotifier *notifier)
{
	if (m_need_update) {
		build(root);
		m_need_update = false;
	}

#ifdef DEBUG_DEPSGRAPH
	std::cerr << "Nodes size: " << m_nodes.size() << '\n';
	std::cerr << "Stack size: " << m_stack.size() << '\n';
#endif

	if (!notifier) {
		/* XXX - TODO: this is mainly to clear objects' cache, which will free
		 * opengl stuff which cacn only be freed from the thread they were
		 * generated. Need a better way to handle caches updates. */
		for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
			DepsNode *node = *iter;
			node->pre_process();
		}
	}

	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		DepsNode *node = *iter;
		node->process(context, notifier);
	}

	context->scene->notify_listeners(-1);
}

const std::vector<DepsNode *> &Depsgraph::nodes() const
{
	return m_nodes;
}

static void gather_nodes(std::vector<DepsNode *> &nodes, DepsNode *root)
{
	if (!root) {
		return;
	}

	nodes.push_back(root);

	for (DepsInputSocket *link : root->output()->links) {
		gather_nodes(nodes, link->parent);
	}
}

static inline auto is_linked(DepsNode *node)
{
	return node->is_linked();
}

static inline auto is_linked(DepsInputSocket *socket)
{
	return socket->link != nullptr;
}

static inline auto get_input(DepsNode *node, size_t /*index*/)
{
	return node->input();
}

static inline auto get_link_parent(DepsInputSocket *socket)
{
	return socket->link->parent;
}

static inline auto num_inputs(DepsNode */*node*/)
{
	return 1;
}

static inline auto get_degree(DepsNode *node)
{
	return node->output()->links.size();
}

static inline auto is_zero_out_degree(DepsNode *node)
{
	return get_degree(node) == 0;
}

void Depsgraph::build(DepsNode *root)
{
	if (root) {
		std::vector<DepsNode *> branch;
		gather_nodes(branch, root);

		topology_sort(branch, m_stack);
	}
	else {
		/* Sort the whole graph. */
		topology_sort(m_nodes, m_stack);
	}
}
