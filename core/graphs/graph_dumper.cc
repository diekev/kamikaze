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

#include "graph_dumper.h"

#include <sstream>

#include "depsgraph.h"
#include "object_graph.h"
#include "object.h"

/* Adapted from Blender's BVM debug code. */

static constexpr auto fontname = "helvetica";
static constexpr auto fontsize = 20.0f;
static constexpr auto node_label_size = 14.0f;
static constexpr auto color_value = "gold1";

inline static std::string node_id(const Node *node, bool quoted = true)
{
	std::stringstream ss;

	ss << "node_" << node;

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

inline int get_input_index(const InputSocket *socket)
{
	auto i = 0;
	for (const auto &input : socket->parent->inputs()) {
		if (input->name == socket->name) {
			return i;
		}

		++i;
	}

	return -1;
}

inline int get_output_index(const OutputSocket *socket)
{
	auto i = 0;
	for (const auto &output : socket->parent->outputs()) {
		if (output->name == socket->name) {
			return i;
		}

		++i;
	}

	return -1;
}

inline static std::string input_id(const InputSocket *socket, int index, bool quoted = true)
{
	if (index == -1) {
		index = get_input_index(socket);
	}

	std::stringstream ss;

	ss << "I" << socket->name << "_" << index;

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

inline static std::string output_id(const OutputSocket *socket, int index, bool quoted = true)
{
	if (index == -1) {
		index = get_output_index(socket);
	}

	std::stringstream ss;

	ss << "O" << socket->name << "_" << index;

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

inline void dump_node(filesystem::File &file, Node *node)
{
	constexpr auto shape = "box";
	constexpr auto style = "filled,rounded";
	constexpr auto color = "black";
	constexpr auto fillcolor = "gainsboro";
	auto penwidth = 1.0f;

	file.print("// %s\n", node->name().c_str());
	file.print("%s", node_id(node).c_str());
	file.print("[");

	file.print("label=<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"4\">");
	file.print("<TR><TD COLSPAN=\"2\">%s</TD></TR>", node->name().c_str());

	const auto numin = node->inputs().size();
	const auto numout = node->outputs().size();

	for (size_t i = 0; (i < numin) || (i < numout); ++i) {
		file.print("<TR>");

		if (i < numin) {
			const auto &input = node->input(i);
			const auto &name_in = input->name;

			file.print("<TD");
			file.print(" PORT=%s", input_id(input, i).c_str());
			file.print(" BORDER=\"1\"");
			file.print(" BGCOLOR=\"%s\"", color_value);
			file.print(">");
			file.print("%s", name_in.c_str());
			file.print("</TD>");
		}
		else {
			file.print("<TD></TD>");
		}

		if (i < numout) {
			const auto &output = node->output(i);
			const auto &name_out = output->name;

			file.print("<TD");
			file.print(" PORT=%s", output_id(output, i).c_str());
			file.print(" BORDER=\"1\"");
			file.print(" BGCOLOR=\"%s\"", color_value);
			file.print(">");
			file.print("%s", name_out.c_str());
			file.print("</TD>");
		}
		else {
			file.print("<TD></TD>");
		}

		file.print("</TR>");
	}

	file.print("</TABLE>>");

	file.print(",fontname=\"%s\"", fontname);
	file.print(",fontsize=\"%f\"", node_label_size);
	file.print(",shape=\"%s\"", shape);
	file.print(",style=\"%s\"", style);
	file.print(",color=\"%s\"", color);
	file.print(",fillcolor=\"%s\"", fillcolor);
	file.print(",penwidth=\"%f\"", penwidth);
	file.print("];\n");
	file.print("\n");
}

inline void dump_link(filesystem::File &file, const OutputSocket *from, const InputSocket *to)
{
	float penwidth = 2.0f;

	file.print("%s:%s -> %s:%s",
	           node_id(from->parent).c_str(), output_id(from, -1).c_str(),
	           node_id(to->parent).c_str(), input_id(to, -1).c_str());

	file.print("[");

	/* Note: without label an id seem necessary to avoid bugs in graphviz/dot */
	file.print("id=\"VAL%s:%s\"", node_id(to->parent, false).c_str(), input_id(to, -1, false).c_str());
	file.print(",penwidth=\"%f\"", penwidth);

	file.print("];\n");
	file.print("\n");
}

inline void dump_node_links(filesystem::File &file, const Node *node)
{
	for (const auto &input : node->inputs()) {
		if (input->link) {
			dump_link(file, input->link, input);
		}
	}
}

GraphDumper::GraphDumper(Graph *graph)
    : m_graph(graph)
{}

void GraphDumper::operator()(const filesystem::path &path)
{
	filesystem::File file(path, "w");

	if (!file) {
		return;
	}

	file.print("digraph depgraph {\n");
	file.print("rankdir=LR\n");
	file.print("graph [");
	file.print("labbelloc=\"t\"");
	file.print(",fontsize=\"%f\"", fontsize);
	file.print("fontname=\"%s\"", fontname);
	file.print("label=\"Object Graph\"");
	file.print("]\n");

	for (const auto &node : m_graph->nodes()) {
		dump_node(file, node);
	}

	for (const auto &node : m_graph->nodes()) {
		dump_node_links(file, node);
	}

	file.print("}\n");
}

/* ************************************************************************** */

#define kmkz_inline static inline

kmkz_inline std::string node_id(const DepsNode *node, bool quoted = true)
{
	std::stringstream ss;

	ss << "node_" << node;

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

kmkz_inline std::string input_id(const DepsInputSocket */*socket*/, bool quoted = true)
{
	std::stringstream ss;

	ss << "IParent";

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

kmkz_inline std::string output_id(const DepsOutputSocket */*socket*/, bool quoted = true)
{
	std::stringstream ss;

	ss << "OChild";

	if (quoted) {
		std::stringstream ssq;
		ssq << std::quoted(ss.str());
		return ssq.str();
	}

	return ss.str();
}

kmkz_inline void dump_node(filesystem::File &file, DepsNode *node)
{
	constexpr auto shape = "box";
	constexpr auto style = "filled,rounded";
	constexpr auto color = "black";
	constexpr auto fillcolor = "gainsboro";
	auto penwidth = 1.0f;

	const auto ob_name = node->name();

	file.print("// %s\n", ob_name);
	file.print("%s", node_id(node).c_str());
	file.print("[");

	file.print("label=<<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"4\">");
	file.print("<TR><TD COLSPAN=\"2\">%s</TD></TR>", ob_name);

	file.print("<TR>");

	const auto &input = node->input();
	const auto &name_in = "Parent";

	file.print("<TD");
	file.print(" PORT=%s", input_id(input).c_str());
	file.print(" BORDER=\"1\"");
	file.print(" BGCOLOR=\"%s\"", color_value);
	file.print(">");
	file.print("%s", name_in);
	file.print("</TD>");

	const auto &output = node->output();
	const auto &name_out = "Child";

	file.print("<TD");
	file.print(" PORT=%s", output_id(output).c_str());
	file.print(" BORDER=\"1\"");
	file.print(" BGCOLOR=\"%s\"", color_value);
	file.print(">");
	file.print("%s", name_out);
	file.print("</TD>");

	file.print("</TR>");

	file.print("</TABLE>>");

	file.print(",fontname=\"%s\"", fontname);
	file.print(",fontsize=\"%f\"", node_label_size);
	file.print(",shape=\"%s\"", shape);
	file.print(",style=\"%s\"", style);
	file.print(",color=\"%s\"", color);
	file.print(",fillcolor=\"%s\"", fillcolor);
	file.print(",penwidth=\"%f\"", penwidth);
	file.print("];\n");
	file.print("\n");
}

kmkz_inline void dump_link(filesystem::File &file,
                           const DepsOutputSocket *from,
                           const DepsInputSocket *to)
{
	float penwidth = 2.0f;

	file.print("%s:%s -> %s:%s",
	           node_id(from->parent).c_str(), output_id(from).c_str(),
	           node_id(to->parent).c_str(), input_id(to).c_str());

	file.print("[");

	/* Note: without label an id seem necessary to avoid bugs in graphviz/dot */
	file.print("id=\"VAL%s:%s\"", node_id(to->parent, false).c_str(), input_id(to, false).c_str());
	file.print(",penwidth=\"%f\"", penwidth);

	file.print("];\n");
	file.print("\n");
}

kmkz_inline void dump_node_links(filesystem::File &file, const DepsNode *node)
{
	for (const auto &output : node->input()->links) {
		dump_link(file, output, node->input());
	}
}

DepsGraphDumper::DepsGraphDumper(Depsgraph *graph)
    : m_graph(graph)
{}

void DepsGraphDumper::operator()(const filesystem::path &path)
{
	filesystem::File file(path, "w");

	if (!file) {
		return;
	}

	file.print("digraph depgraph {\n");
	file.print("rankdir=LR\n");
	file.print("graph [");
	file.print("labbelloc=\"t\"");
	file.print(",fontsize=\"%f\"", fontsize);
	file.print("fontname=\"%s\"", fontname);
	file.print("label=\"Dependency Graph\"");
	file.print("]\n");

	for (const auto &node : m_graph->nodes()) {
		dump_node(file, node);
	}

	for (const auto &node : m_graph->nodes()) {
		dump_node_links(file, node);
	}

	file.print("}\n");
}
