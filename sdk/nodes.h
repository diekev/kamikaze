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

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "persona.h"

class EvaluationContext;
class InputSocket;
class Node;
class NodeFactory;
class ParamCallback;
class Primitive;
class PrimitiveCache;
class PrimitiveCollection;

extern "C" {

/**
 * @brief new_kamikaze_node API for registering a new node from a plugin.
 *                          There is no limit to the number of nodes to register
 *                          from a single call to this function.
 * @param factory The factory in which to register the node(s).
 */
void new_kamikaze_node(NodeFactory *factory);

}

/**
 * Macro to help registering nodes.
 */
#define REGISTER_NODE(category, name, type) \
	factory->registerType(category, name, []() -> Node* { return new type; })

struct OutputSocket {
	Node *parent = nullptr;
	std::vector<InputSocket *> links{};
	std::string name = "";
	PrimitiveCollection *collection = nullptr;

	explicit OutputSocket(const std::string &sname)
	    : parent(nullptr)
	    , name(sname)
	{}
};

struct InputSocket {
	Node *parent = nullptr;
	OutputSocket *link = nullptr;
	std::string name = "";
	PrimitiveCollection *collection = nullptr;

	explicit InputSocket(const std::string &sname)
	    : parent(nullptr)
	    , link(nullptr)
	    , name(sname)
	{}
};

/* ********************************** */

class Node : public Persona {
protected:
	std::vector<InputSocket *> m_inputs = {};
	std::vector<OutputSocket *> m_outputs = {};
	std::string m_name = "";
	std::string m_icon_path = "";
	PrimitiveCache *m_cache = nullptr;
	PrimitiveCollection *m_collection = nullptr;
	std::vector<std::string> m_warnings;

public:
	explicit Node(const std::string &name);
	Node(const Node &other) = default;

	virtual ~Node();

	/**
	 * Return the name of this node.
	 */
	const std::string &name() const noexcept;

	/**
	 * Add an input to the node.
	 */
	void addInput(const std::string &sname);

	/**
	 * Add an output to the node.
	 */
	void addOutput(const std::string &sname);

	/**
	 * Return the input at the given index.
	 */
	InputSocket *input(int index);

	/**
	 * Return the input with the given name.
	 */
	InputSocket *input(const std::string &name);

	/**
	 * Return the output at the given index.
	 */
	OutputSocket *output(int index);

	/**
	 * Return the output with the given name.
	 */
	OutputSocket *output(const std::string &name);

	/**
	 * Return the vector of inputs of this node.
	 */
	std::vector<InputSocket *> inputs() const noexcept;

	/**
	 * Return the vector of outputs of this node.
	 */
	std::vector<OutputSocket *> outputs() const noexcept;

	/**
	 * Return whether or not this node is linked.
	 */
	bool isLinked() const;

	/**
	 * Return whether or not this node has a linked input.
	 */
	bool hasLinkedInput() const;

	/**
	 * Return whether or not this node has a linked output.
	 */
	bool hasLinkedOutput() const;

	/**
	 * Process the node.
	 */
	virtual void process() = 0;

	/**
	 * Get the collection at the given input name.
	 */
	PrimitiveCollection *getInputCollection(const std::string &name);

	/**
	 * Get the collection at the given input index.
	 */
	PrimitiveCollection *getInputCollection(const size_t index);

	/**
	 * Set the collection at the given output name.
	 */
	void setOutputCollection(const std::string &name, PrimitiveCollection *collection);

	/**
	 * Set the collection at the given output index.
	 */
	void setOutputCollection(const size_t index, PrimitiveCollection *collection);

	/**
	 * Return this node's collection.
	 */
	PrimitiveCollection *collection() const;

	/**
	 * Set this node's collection.
	 */
	void collection(PrimitiveCollection *coll);

	/**
	 * Set the primitive cache.
	 */
	void setPrimitiveCache(PrimitiveCache *cache);

	/**
	 * Set this node's icon path.
	 */
	void icon_path(const std::string &path);

	/**
	 * Get this node's icon path.
	 */
	std::string icon_path() const;

	/**
	 * Add a warning to the node's warning list.
	 */
	void add_warning(const std::string &warning);

	/**
	 * Return this node's warning list.
	 */
	const std::vector<std::string> &warnings() const;

	/**
	 * Return whether this node has warnings or not.
	 */
	bool has_warning() const;

	/**
	 * Clear this node's warning list.
	 */
	void clear_warnings();

private:
	/**
	 * Get the collection at the given input socket.
	 */
	PrimitiveCollection *getInputCollection(InputSocket *socket);

	/**
	 * Set the collection at the given output socket.
	 */
	void setOutputCollection(OutputSocket *socket, PrimitiveCollection *collection);
};

class NodeFactory final {
public:
	typedef Node *(*factory_func)(void);

	void registerType(const std::string &category, const std::string &name, factory_func func);

	Node *operator()(const std::string &name);

	size_t numEntries() const;

	std::vector<std::string> keys(const std::string &category) const;

	std::vector<std::string> categories() const;

private:
	std::unordered_map<std::string, factory_func> m_map;
	std::unordered_map<std::string, std::vector<std::string>> m_cat_map;
};
