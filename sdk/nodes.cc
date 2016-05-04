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

#include "nodes.h"

#include <cassert>

#include "../core/object.h"  /* XXX - bad level call */
#include "primitive.h"

Node::Node(const std::string &name)
    : m_name(name)
{}

Node::~Node()
{
	for (auto &input : m_inputs) {
		delete input;
	}

	for (auto &output : m_outputs) {
		delete output;
	}
}

const std::string &Node::name() const noexcept
{
	return m_name;
}

void Node::addInput(const std::string &sname)
{
	InputSocket *in = new InputSocket(sname);
	in->parent = this;
	m_inputs.push_back(in);
}

void Node::addOutput(const std::string &sname)
{
	OutputSocket *out = new OutputSocket(sname);
	out->parent = this;
	m_outputs.push_back(out);
}

InputSocket *Node::input(int index)
{
	return m_inputs[index];
}

InputSocket *Node::input(const std::string &name)
{
	for (const auto &input : m_inputs) {
		if (input->name == name) {
			return input;
		}
	}

	return nullptr;
}

OutputSocket *Node::output(int index)
{
	return m_outputs[index];
}

OutputSocket *Node::output(const std::string &name)
{
	for (const auto &output : m_outputs) {
		if (output->name == name) {
			return output;
		}
	}

	return nullptr;
}

std::vector<InputSocket *> Node::inputs() const noexcept
{
	return m_inputs;
}

std::vector<OutputSocket *> Node::outputs() const noexcept
{
	return m_outputs;
}

bool Node::isLinked() const
{
	return hasLinkedInput() || hasLinkedOutput();
}

bool Node::hasLinkedInput() const
{
	bool linked_input = false;

	for (const auto &input : m_inputs) {
		if (input->link != nullptr) {
			linked_input = true;
			break;
		}
	}

	return linked_input;
}

bool Node::hasLinkedOutput() const
{
	bool linked_output = false;

	for (const auto &output : m_outputs) {
		if (!output->links.empty()) {
			linked_output = true;
			break;
		}
	}

	return linked_output;
}

Primitive *Node::getInputPrimitive(const std::string &name)
{
	auto socket = input(name);

	if (!socket || !socket->link) {
		return nullptr;
	}

	auto prim = socket->link->prim;

	if (!prim) {
		return nullptr;
	}

	if (socket->link->links.size() > 1) {
		auto copy = prim->copy();

		if (!copy) {
			return nullptr;
		}

		m_cache->add(copy);
		copy->incref();

		return copy;
	}

	return prim;
}

void Node::setOutputPrimitive(const std::string &name, Primitive *prim)
{
	auto socket = output(name);

	if (prim) {
		m_cache->add(prim);
		prim->incref();
	}

	if (!socket) {
		return;
	}

	socket->prim = prim;
}

void Node::setPrimitiveCache(PrimitiveCache *cache)
{
	m_cache = cache;
}

/* ****************************** node factory ****************************** */

void NodeFactory::registerType(const std::string &category, const std::string &name, NodeFactory::factory_func func)
{
	const auto iter = m_map.find(name);

	assert(iter == m_map.end());

	m_map[name] = func;

	auto cat_iter = m_cat_map.find(category);

	if (cat_iter == m_cat_map.end()) {
		std::vector<std::string> v;
		v.push_back(name);
		m_cat_map[category] = v;
	}
	else {
		(m_cat_map[category]).push_back(name);
	}
}

Node *NodeFactory::operator()(const std::string &name)
{
	const auto iter = m_map.find(name);
	assert(iter != m_map.end());

	return iter->second();
}

size_t NodeFactory::numEntries() const
{
	return m_map.size();
}

std::vector<std::string> NodeFactory::keys(const std::string &category) const
{
	const auto iter = m_cat_map.find(category);

	assert(iter != m_cat_map.end());

	return iter->second;
}

std::vector<std::string> NodeFactory::categories() const
{
	std::vector<std::string> v;

	for (const auto &entry : m_cat_map) {
		v.push_back(entry.first);
	}

	return v;
}
