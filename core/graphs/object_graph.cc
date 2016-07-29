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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "object_graph.h"

#include <algorithm>
#include <iostream>

#include "graph_dumper.h"
#include "object_nodes.h"

//#define DEBUG_GRAPH

Graph::Graph()
    : m_need_update(false)
{
	add(new OutputNode("Output"));
}

Graph::~Graph()
{
	for (auto &node : m_nodes) {
		delete node;
	}
}

const std::vector<Node *> &Graph::nodes() const
{
	return m_nodes;
}

void Graph::add(Node *node)
{
	m_nodes.push_back(node);
}

void Graph::remove(Node *node)
{
	auto iter = std::find(m_nodes.begin(), m_nodes.end(), node);

	if (iter == m_nodes.end()) {
		std::cerr << "Unable to find node in graph!\n";
		return;
	}

	/* disconnect inputs */
	for (InputSocket *input : node->inputs()) {
		if (input->link) {
			disconnect(input->link, input);
		}
	}

	/* disconnect outputs */
	for (OutputSocket *output : node->outputs()) {
		for (InputSocket *input : output->links) {
			disconnect(output, input);
		}
	}

	delete node;
	m_nodes.erase(iter);

	m_need_update = true;
}

void Graph::build()
{
	if (!m_need_update) {
		return;
	}

	topology_sort();

#ifdef DEBUG_GRAPH
	std::cerr << "Order of operation:\n";

	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		Node *node = *iter;
		std::cerr << node->name() << '\n';
	}
#endif

	m_need_update = false;
}

void Graph::topology_sort()
{
	m_stack.clear();
	m_stack.reserve(m_nodes.size());

	/* 1. store each node degree in an array */
	std::vector<Node *> stack;
	std::unordered_map<Node *, std::pair<int, bool>> node_degrees;

	int degree;

	for (Node *node : m_nodes) {
		if (!node->isLinked()) {
			continue;
		}

		/* 2. initialize a stack with all out-degree zero nodes */
		if (node->outputs().size() == 0) {
			stack.push_back(node);
			node_degrees[node] = std::make_pair(-1, true);
			continue;
		}

		degree = 0;

		for (OutputSocket *socket : node->outputs()) {
			degree += socket->links.size();
		}

		node_degrees[node] = std::make_pair(degree, false);
	}

	/* 3. While there are vertices remaining in the stack:
	 *   - Dequeue and output a node
	 *   - Reduce out-degree of all vertices adjacent to it by 1
	 *   - Enqueue any of these nodes whose out-degree became zero
	 */
	Node *node;
	while (!stack.empty()) {
		node = stack.back();
		m_stack.push_back(node);
		stack.pop_back();

		for (InputSocket *socket : node->inputs()) {
			if (!socket->link) {
				continue;
			}

			auto &degree_pair = node_degrees[socket->link->parent];

			if (degree_pair.second) {
				continue;
			}

			degree_pair.first -= 1;

			if (degree_pair.first == 0) {
				stack.push_back(socket->link->parent);
				degree_pair.second = true;
			}
		}
	}
}

const std::vector<Node *> &Graph::finished_stack() const
{
	return m_stack;
}

OutputNode *Graph::output() const
{
	return static_cast<OutputNode *>(m_nodes.front());
}

void Graph::connect(OutputSocket *from, InputSocket *to)
{
	if (to->link != nullptr) {
		std::cerr << "Input already connected!\n";
		return;
	}

	to->link = from;
	from->links.push_back(to);

	m_need_update = true;
}

void Graph::disconnect(OutputSocket *from, InputSocket *to)
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

void Graph::active_node(Node *node)
{
	m_active_node = node;
}

Node *Graph::active_node() const
{
	return m_active_node;
}
