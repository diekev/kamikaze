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

#pragma once

/* Perform a topological sort of the nodes in a directed acyclic graph.
 * This is templated over the node type and all of the functions called in here
 * are to be overloaded with the right node type. */
template <typename NodeType>
static void topology_sort(const std::vector<NodeType *> &nodes, std::vector<NodeType *> &r_stack)
{
	std::vector<NodeType *> sorted_stack;
	sorted_stack.reserve(nodes.size());

	/* 1. Store each node degree in an array. */
	std::vector<NodeType *> stack;
	std::unordered_map<NodeType *, std::pair<int, bool>> node_degrees;

	int degree;

	for (auto node : nodes) {
		if (!is_linked(node)) {
			continue;
		}

		/* 2. initialize a stack with all out-degree zero nodes */

		if (is_zero_out_degree(node)) {
			stack.push_back(node);
			node_degrees[node] = std::make_pair(-1, true);
		}
		else {
			degree = get_degree(node);
			node_degrees[node] = std::make_pair(degree, false);
		}
	}

	/* 3. While there are vertices remaining in the stack... */
	while (!stack.empty()) {
		/* 3a. ...dequeue and output a node... */
		auto node = stack.back();
		sorted_stack.push_back(node);
		stack.pop_back();

		/* Get vertices adjacent to this vertex. */
		for (size_t i = 0, e = num_inputs(node); i < e; ++i) {
			auto socket = get_input(node, i);

			/* This input is not linked, skip. */
			if (!is_linked(socket)) {
				continue;
			}

			/* Get vertex from which this link comes. */
			auto parent = get_link_parent(socket);
			auto &degree_pair = node_degrees[parent];

			/* If already enqueued, skip. */
			if (degree_pair.second) {
				continue;
			}

			/* 3b. ...reduce out-degree of adjacent vertex by 1... */
			degree_pair.first -= 1;

			/* 3c. ...enqueue vertex if out-degree became zero. */
			if (degree_pair.first == 0) {
				stack.push_back(parent);
				degree_pair.second = true;
			}
		}
	}

	r_stack = sorted_stack;
}
