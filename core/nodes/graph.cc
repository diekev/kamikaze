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

#include "graph.h"

#include <algorithm>
#include <iostream>

#include "graph_dumper.h"

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

void Graph::build()
{
	if (!m_need_update) {
		return;
	}

	m_stack.clear();
	m_stack.reserve(m_nodes.size());

	topology_sort();

#ifdef DEBUG_GRAPH
	std::cerr << "Order of operation:\n";

	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		Node *node = *iter;
		std::cerr << node->name() << '\n';
	}

	GraphDumper gd(this);
	gd("/tmp/kamikaze.gv");
#endif

	m_need_update = false;
}

void Graph::topology_sort()
{
	/* 1. store each node degree in an array */
	std::vector<Node *> stack;

	std::vector<int> degrees(m_nodes.size());
	std::vector<bool> resolved(m_nodes.size());
	Node *node;
	int degree;

	for (size_t i = 0; i < m_nodes.size(); ++i) {
		node = m_nodes[i];
		degree = 0;

		/* 2. initialize a stack with all out-degree zero nodes */
		if (node->outputs().size() == 0) {
			stack.push_back(m_nodes[i]);
			degrees[i] = -1;
			resolved[i] = true;
			continue;
		}

		for (OutputSocket *socket : node->outputs()) {
			degree += socket->links.size();
		}

		degrees[i] = degree;
		resolved[i] = false;
	}

	/* 3. While there are vertices remaining in the stack:
	 *   - Dequeue and output a node
	 *   - Reduce out-degree of all vertices adjacent to it by 1
	 *   - Enqueue any of these nodes whose out-degree became zero
	 */
	while (!stack.empty()) {
		node = stack.back();
		m_stack.push_back(node);
		stack.pop_back();

		for (InputSocket *socket : node->inputs()) {
			if (!socket->link) {
				continue;
			}

			auto iter = std::find(m_nodes.begin(), m_nodes.end(), socket->link->parent);

			if (iter == m_nodes.end()) {
				continue;
			}

			auto index = std::distance(m_nodes.begin(), iter);

			if (resolved[index]) {
				continue;
			}

			degrees[index] -= 1;

			if (degrees[index] == 0) {
				stack.push_back(m_nodes[index]);
				resolved[index] = true;
			}
		}
	}
}

void Graph::execute()
{
	for (auto iter = m_stack.rbegin(); iter != m_stack.rend(); ++iter) {
		Node *node = *iter;
		node->process();
	}
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

/* ****************************** output node ******************************* */

OutputNode::OutputNode(const std::string &name)
    : Node(name)
{
	addInput("Primitive");
}

Primitive *OutputNode::primitive() const
{
	return m_primitive;
}

void OutputNode::process()
{
	m_primitive = getInputPrimitive("Primitive");
}

void OutputNode::setUIParams(ParamCallback *)
{
}
