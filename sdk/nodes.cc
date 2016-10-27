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

#include "context.h"
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

void Node::name(const std::string &new_name)
{
	m_name = new_name;
}

const std::string &Node::name() const noexcept
{
	return m_name;
}

float Node::xpos() const
{
	return m_xpos;
}

void Node::xpos(float x)
{
	m_xpos = x;
}

float Node::ypos() const
{
	return m_ypos;
}

void Node::ypos(float y)
{
	m_ypos = y;
}

void Node::addInput(const std::string &sname)
{
	auto in = new InputSocket(sname);
	in->parent = this;
	m_inputs.push_back(in);
}

void Node::addOutput(const std::string &sname)
{
	auto out = new OutputSocket(sname);
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
	auto linked_input = false;

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
	auto linked_output = false;

	for (const auto &output : m_outputs) {
		if (!output->links.empty()) {
			linked_output = true;
			break;
		}
	}

	return linked_output;
}

PrimitiveCollection *Node::getInputCollection(const std::string &name)
{
	auto socket = input(name);
	return getInputCollection(socket);
}

PrimitiveCollection *Node::getInputCollection(const size_t index)
{
	auto socket = input(index);
	return getInputCollection(socket);
}

void Node::setOutputCollection(const std::string &name, PrimitiveCollection *collection)
{
	auto socket = output(name);
	setOutputCollection(socket, collection);
}

void Node::setOutputCollection(const size_t index, PrimitiveCollection *collection)
{
	auto socket = output(index);
	setOutputCollection(socket, collection);
}

PrimitiveCollection *Node::collection() const
{
	return m_collection;
}

void Node::collection(PrimitiveCollection *coll)
{
	m_collection = coll;
}

void Node::setPrimitiveCache(PrimitiveCache *cache)
{
	m_cache = cache;
}

void Node::icon_path(const std::string &path)
{
	m_icon_path = path;
}

std::string Node::icon_path() const
{
	return m_icon_path;
}

void Node::add_warning(const std::string &warning)
{
	m_warnings.push_back(warning);
}

const std::vector<std::string> &Node::warnings() const
{
	return m_warnings;
}

bool Node::has_warning() const
{
	return (m_warnings.empty() == false);
}

void Node::clear_warnings()
{
	m_warnings.clear();
}

PrimitiveCollection *Node::getInputCollection(InputSocket *socket)
{
	if (!socket || !socket->link) {
		return nullptr;
	}

	auto collection = socket->link->collection;

	if (!collection) {
		return nullptr;
	}

	if (socket->link->links.size() > 1) {
		auto copy = collection->copy();

		if (!copy) {
			return nullptr;
		}

		m_cache->add(copy);

		return copy;
	}

	return collection;
}

void Node::setOutputCollection(OutputSocket *socket, PrimitiveCollection *collection)
{
	if (collection) {
		m_cache->add(collection);
	}

	if (!socket) {
		return;
	}

	socket->collection = collection;
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

void swap(any &&lhs, any &&rhs)
{
	lhs.swap(rhs);
}
