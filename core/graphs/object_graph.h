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

#include <kamikaze/nodes.h>
#include <kamikaze/primitive.h>
#include <memory>
#include <vector>

class InputSocket;
class OutputNode;
class OutputSocket;

enum {
	NODE_SELECTED = (1 << 0),
};

class Graph {
	std::vector<std::unique_ptr<Node>> m_nodes;
	std::vector<Node *> m_stack;
	std::vector<Node *> m_selected_nodes;

	Node *m_active_node = nullptr;

	PrimitiveCache m_cache;

	bool m_need_update;

public:
	Graph();
	~Graph();

	void add(Node *node);
	void remove(Node *node);

	void connect(OutputSocket *from, InputSocket *to);
	void disconnect(OutputSocket *from, InputSocket *to);

	void build();

	OutputNode *output() const;

	const std::vector<std::unique_ptr<Node> > &nodes() const;
	const std::vector<Node *> &finished_stack() const;

	void active_node(Node *node);
	Node *active_node() const;

	void clear_cache();

	void add_to_selection(Node *node);

	void remove_from_selection(Node *node);
};
