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

#include "scene_node.h"

SceneNode::~SceneNode()
{
	for (auto &input : m_inputs) {
		delete input;
	}

	for (auto &output : m_outputs) {
		delete output;
	}
}

void SceneNode::add_input(const std::string &name)
{
	auto socket = new SceneInputSocket(name);
	socket->parent = this;

	this->m_inputs.push_back(socket);
}

void SceneNode::add_output(const std::string &name)
{
	auto socket = new SceneOutputSocket(name);
	socket->parent = this;

	this->m_outputs.push_back(socket);
}

const std::vector<SceneInputSocket *> &SceneNode::inputs()
{
	return m_inputs;
}

const std::vector<SceneOutputSocket *> &SceneNode::outputs()
{
	return m_outputs;
}

void SceneNode::name(const std::string &name)
{
	m_name = name;
}

const std::string &SceneNode::name() const
{
	return m_name;
}
