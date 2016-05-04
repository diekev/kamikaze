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

#include <glm/glm.hpp>

#include <kamikaze/nodes.h>

void register_builtin_nodes(NodeFactory *factory);

class OutputNode : public Node {
	Primitive *m_primitive = nullptr;

public:
	OutputNode(const std::string &name);

	Primitive *primitive() const;

	void process() override;

	void setUIParams(ParamCallback *cb) override;
};

class TransformNode : public Node {
	glm::vec3 m_translate = glm::vec3{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_rotate = glm::vec3{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_scale = glm::vec3{ 1.0f, 1.0f, 1.0f };
	glm::vec3 m_pivot = glm::vec3{ 0.0f, 0.0f, 0.0f };
	float m_uniform_scale = 1.0f;
	bool m_invert = false;
	int m_transform_type = 0;
	int m_rot_order = 0;

public:
	TransformNode();

	void process() override;

	void setUIParams(ParamCallback *cb) override;
};

class CreateBoxNode : public Node {
	glm::vec3 m_size = glm::vec3{ 1.0f, 1.0f, 1.0f };
	glm::vec3 m_center = glm::vec3{ 0.0f, 0.0f, 0.0f };
	float m_uniform_scale = 1.0f;

public:
	CreateBoxNode();

	void process() override;

	void setUIParams(ParamCallback *cb) override;
};

class CreateTorusNode : public Node {
	glm::vec3 m_center = glm::vec3{ 0.0f, 0.0f, 0.0f };

	float m_major_radius = 1.0f;
	float m_minor_radius = 0.25f;
	int m_major_segment = 48;
	int m_minor_segment = 24;

	float m_uniform_scale = 1.0f;

public:
	CreateTorusNode();

	void process() override;

	void setUIParams(ParamCallback *cb) override;
};

class CreateGridNode : public Node {
	glm::vec3 m_size = glm::vec3{ 1.0f, 1.0f, 0.0f };
	glm::vec3 m_center = glm::vec3{ 0.0f, 0.0f, 0.0f };

	int m_rows = 2;
	int m_columns = 2;

public:
	CreateGridNode();

	void process() override;

	void setUIParams(ParamCallback *cb) override;
};
