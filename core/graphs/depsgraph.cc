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
	return (!m_input.links.empty()) || (!m_output.links.empty());
}

/* ************************************************************************** */

DepsObjectNode::DepsObjectNode(Object *object)
    : m_object(object)
{}

void DepsObjectNode::pre_process()
{
	/* TODO: what's the purpose of this again? */
	m_object->collection(nullptr);
}

void DepsObjectNode::process(const Context & /*context*/, TaskNotifier */*notifier*/)
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
	return m_object->name().c_str();
}

/* ************************************************************************** */

ObjectGraphDepsNode::ObjectGraphDepsNode(Graph *graph)
    : m_graph(graph)
{}

void ObjectGraphDepsNode::pre_process()
{
	m_graph->clear_cache();
}

void ObjectGraphDepsNode::process(const Context &context, TaskNotifier *notifier)
{
	auto output_node = m_graph->output();

	if (!output_node->isLinked()) {
		output_node->input(0)->collection = nullptr;

		/* Make sure the node's collection is updated, otherwise can crash. */
		output_node->process();
		return;
	}

	m_graph->build();

	/* XXX */
	for (const auto &node : m_graph->nodes()) {
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
		PrimitiveCollection *collection = nullptr;

		if (node->inputs().empty()) {
			collection = new PrimitiveCollection(context.primitive_factory);
		}
		else {
			collection = node->getInputCollection(0ul);
		}

		node->collection(collection);

		/* Make sure warnings are cleared before processing. */
		node->clear_warnings();

		if (node->collection()) {
			node->process();
		}

		if (!node->outputs().empty()) {
			node->setOutputCollection(0ul, node->collection());
		}

		if (notifier) {
			const float progress = (++index / size) * 100.0f;
			notifier->signalProgressUpdate(progress);

			/* To refresh the UI in case new warnings appear. */
			if (context.eval_ctx->edit_mode && node == m_graph->active_node()) {
				notifier->signalNodeProcessed();
			}
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

void TimeDepsNode::process(const Context & /*context*/, TaskNotifier */*notifier*/)
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
	GraphEvalTask(Depsgraph *graph, const Context &context, DepsNode *root);

	void start(const Context &context) override;
};

GraphEvalTask::GraphEvalTask(Depsgraph *graph, const Context &context, DepsNode *root)
    : Task(context)
    , m_graph(graph)
    , m_root(root)
{}

void GraphEvalTask::start(const Context &context)
{
	m_graph->evaluate_ex(context, m_root, m_notifier.get());
}

/* ************************************************************************** */

Depsgraph::Depsgraph()
    : m_time_node(new TimeDepsNode)
{
	m_nodes.emplace_back(m_time_node);
}

void Depsgraph::connect(SceneNode *from, SceneNode *to)
{
	auto from_deps_node = find_node(from, false);
	auto to_deps_node = find_node(to, true);

	connect(from_deps_node->output(), to_deps_node->input());
}

void Depsgraph::disconnect(SceneNode *from, SceneNode *to)
{
	auto from_deps_node = find_node(from, false);
	auto to_deps_node = find_node(to, true);

	disconnect(from_deps_node->output(), to_deps_node->input());
}

DepsNode *Depsgraph::find_node(SceneNode *scene_node, bool graph)
{
	DepsNode *node = nullptr;

	/* TODO: find a better way for this. */
	auto object = static_cast<Object *>(scene_node);
	auto iter = m_object_graph_map.find(object->graph());
	assert(iter != m_object_graph_map.end());

	node = iter->second;

	assert(node != nullptr);

	return node;
}

void Depsgraph::connect(DepsOutputSocket *from, DepsInputSocket *to)
{
	to->links.push_back(from);
	from->links.push_back(to);

	m_need_update = true;
}

void Depsgraph::disconnect(DepsOutputSocket *from, DepsInputSocket *to)
{
	{
		auto iter = std::find(from->links.begin(), from->links.end(), to);

		if (iter == from->links.end()) {
			std::cerr << "Depsgraph::disconnect, cannot find output!\n";
			return;
		}

		from->links.erase(iter);
	}

	{
		auto iter = std::find(to->links.begin(), to->links.end(), from);

		if (iter == to->links.end()) {
			std::cerr << "Depsgraph::disconnect, cannot find input!\n";
			return;
		}

		to->links.erase(iter);
	}

	m_need_update = true;
}

void Depsgraph::create_node(SceneNode *scene_node)
{
	auto object = static_cast<Object *>(scene_node);
	m_nodes.push_back(std::unique_ptr<DepsNode>(new DepsObjectNode(object)));
	auto node = m_nodes.back().get();

	m_scene_node_map[scene_node] = node;

	m_nodes.push_back(std::unique_ptr<DepsNode>(new ObjectGraphDepsNode(object->graph())));
	auto graph_node = m_nodes.back().get();

	m_object_graph_map[object->graph()] = graph_node;

	/* Object depends on its graph. */
	connect(graph_node->output(), node->input());

	m_need_update = true;
}

void Depsgraph::remove_node(SceneNode *scene_node)
{
	/* First, remove graph node. */
	{
		auto object = static_cast<Object *>(scene_node);
		auto iter = m_object_graph_map.find(object->graph());
		assert(iter != m_object_graph_map.end());

		DepsNode *node = iter->second;

		auto node_iter = std::find_if(m_nodes.begin(), m_nodes.end(),
		                              [&node](const std::unique_ptr<DepsNode> &node_ptr)
		{
			return node_ptr.get() == node;
		});
		assert(node_iter != m_nodes.end());

		/* Disconnect input. */
		for (DepsOutputSocket *output : node->input()->links) {
			disconnect(output, node->input());
		}

		/* Disconnect output. */
		for (DepsInputSocket *input : node->output()->links) {
			disconnect(node->output(), input);
		}

		m_object_graph_map.erase(iter);
		m_nodes.erase(node_iter);
	}

	/* Then, remove scene node. */
	{
		auto iter = m_scene_node_map.find(scene_node);
		assert(iter != m_scene_node_map.end());

		DepsNode *node = iter->second;

		auto node_iter = std::find_if(m_nodes.begin(), m_nodes.end(),
		                              [&node](const std::unique_ptr<DepsNode> &node_ptr)
		{
			return node_ptr.get() == node;
		});
		assert(node_iter != m_nodes.end());

		/* Disconnect input. */
		for (DepsOutputSocket *output : node->input()->links) {
			disconnect(output, node->input());
		}

		/* Disconnect output. */
		for (DepsInputSocket *input : node->output()->links) {
			disconnect(node->output(), input);
		}

		m_scene_node_map.erase(iter);
		m_nodes.erase(node_iter);
	}

	m_need_update = true;
}

void Depsgraph::connect_to_time(SceneNode *scene_node)
{
	auto node = find_node(scene_node, true);
	connect(m_time_node->output(), node->input());
}

void Depsgraph::evaluate(const Context &context, SceneNode *scene_node)
{
	auto node = find_node(scene_node, true);

	m_need_update |= (m_state != DEG_STATE_OBJECT);
	m_state = DEG_STATE_OBJECT;

	GraphEvalTask *t = new(tbb::task::allocate_root()) GraphEvalTask(this, context, node);
	tbb::task::enqueue(*t);
}

void Depsgraph::evaluate_for_time_change(const Context &context)
{
	m_need_update |= (m_state != DEG_STATE_TIME);
	m_state = DEG_STATE_TIME;

	evaluate_ex(context, m_time_node, nullptr);
}

void Depsgraph::evaluate_ex(const Context &context, DepsNode *root, TaskNotifier *notifier)
{
	if (m_need_update) {
		build(root);
		m_need_update = false;
	}

#ifdef DEBUG_DEPSGRAPH
	std::cerr << "Nodes size: " << m_nodes.size() << '\n';
	std::cerr << "Stack size: " << m_stack.size() << '\n';
#endif

	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		DepsNode *node = *iter;
		node->pre_process();
	}

	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		DepsNode *node = *iter;
		node->process(context, notifier);
	}

	context.scene->notify_listeners(static_cast<event_type>(-1));
}

const std::vector<std::unique_ptr<DepsNode>> &Depsgraph::nodes() const
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
	return !socket->links.empty();
}

static inline auto get_input(DepsNode *node, size_t /*index*/)
{
	return node->input();
}

static inline auto get_link_parent(DepsInputSocket *socket)
{
	return socket->links[0]->parent;
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
		std::vector<DepsNode *> nodes(m_nodes.size());

		std::transform(m_nodes.begin(), m_nodes.end(), nodes.begin(),
		               [](const std::unique_ptr<DepsNode> &node) -> DepsNode*
		{
			return node.get();
		});

		topology_sort(nodes, m_stack);
	}
}
