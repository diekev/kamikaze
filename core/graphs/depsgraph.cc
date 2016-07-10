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

void DepsObjectNode::process() {}

Object *DepsObjectNode::object()
{
	return m_object;
}

const Object *DepsObjectNode::object() const
{
	return m_object;
}

/* ************************************************************************** */

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

	m_need_update = true;
}

void Depsgraph::remove_node(Object *object)
{
	auto iter = m_object_map.find(object);
	assert(iter != m_object_map.end());

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

	m_object_map.erase(iter);
	m_nodes.erase(node_iter);
	delete node;

	m_need_update = true;
}

void Depsgraph::evaluate(const EvaluationContext * const)
{
	if (m_need_update) {
		topology_sort();
		m_need_update = false;
	}

	for (DepsNode *node : m_nodes) {
		node->process();
	}
}

const std::vector<DepsNode *> &Depsgraph::nodes() const
{
	return m_nodes;
}

void Depsgraph::topology_sort()
{
	m_stack.clear();
	m_stack.reserve(m_nodes.size());

	/* 1. store each node degree in an array */
	std::vector<DepsNode *> stack;
	std::unordered_map<DepsNode *, std::pair<int, bool>> node_degrees;

	int degree;

	for (DepsNode *node : m_nodes) {
		if (!node->is_linked()) {
			m_stack.push_back(node);
			continue;
		}

		/* 2. initialize a stack with all out-degree zero nodes */

		degree = node->output()->links.size();

		if (degree == 0) {
			stack.push_back(node);
			node_degrees[node] = std::make_pair(-1, true);
		}
		else {
			node_degrees[node] = std::make_pair(degree, false);
		}
	}

	/* 3. While there are vertices remaining in the stack:
	 *   - Dequeue and output a node
	 *   - Reduce out-degree of all vertices adjacent to it by 1
	 *   - Enqueue any of these nodes whose out-degree became zero
	 */
	while (!stack.empty()) {
		DepsNode *node = stack.back();
		m_stack.push_back(node);
		stack.pop_back();

		DepsInputSocket *socket = node->input();

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
