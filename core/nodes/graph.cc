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

void Graph::build(Node *node)
{
	auto iter = std::find(m_stack.begin(), m_stack.end(), node);

	/* same node was connected to other node, move it to the "front" */
	if (iter != m_stack.end()) {
		std::rotate(iter, iter + 1, m_stack.end());
	}
	else {
		m_stack.push_back(node);
	}

	if (!node->isLinked()) {
		return;
	}

	for (const auto &input : node->inputs()) {
		if (input->link != nullptr) {
			build(input->link->parent);
		}
	}
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

	m_stack.reserve(m_nodes.size());
	build(m_nodes.front());

#if 0
	std::cerr << "Number of nodes: " << m_nodes.size() << "\n";
	std::cerr << "Stack size: " << m_stack.size() << "\n";
#endif

	m_need_update = false;
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
