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
#include <vector>

class InputSocket;
class OutputNode;
class OutputSocket;

class Graph {
	std::vector<Node *> m_nodes;
	std::vector<Node *> m_stack;

	bool m_need_update;

	void topology_sort();

public:
	Graph();
	~Graph();

	void add(Node *node);
	void connect(OutputSocket *from, InputSocket *to);
	void disconnect(OutputSocket *from, InputSocket *to);

	void build();
	void execute();

	OutputNode *output() const;

	const std::vector<Node *> &nodes() const;
};

class OutputNode : public Node {
	Primitive *m_primitive = nullptr;

public:
	OutputNode(const std::string &name);

	Primitive *primitive() const;

	void process() override;

	void setUIParams(ParamCallback */*cb*/) override;
};
