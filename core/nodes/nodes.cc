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

#include "nodes.h"

#include <kamikaze/mesh.h>
#include <kamikaze/paramfactory.h>
#include <kamikaze/primitive.h>

#include "util/utils_glm.h"

/* ************************************************************************** */

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

/* ************************************************************************** */

TransformNode::TransformNode()
    : Node("Transform")
{
	addInput("Prim");
	addOutput("Prim");
}

void TransformNode::process()
{
	auto prim = getInputPrimitive("Prim");

	if (!prim) {
		setOutputPrimitive("Prim", nullptr);
		return;
	}

	auto scale = m_scale * m_uniform_scale;

	/* determine the rotatation order */
	int rot_ord[6][3] = {
	    { 0, 1, 2 }, // X Y Z
	    { 0, 2, 1 }, // X Z Y
	    { 1, 0, 2 }, // Y X Z
	    { 1, 2, 0 }, // Y Z X
	    { 2, 0, 1 }, // Z X Y
	    { 2, 1, 0 }, // Z Y X
	};

	glm::vec3 axis[3] = {
	    glm::vec3(1.0f, 0.0f, 0.0f),
	    glm::vec3(0.0f, 1.0f, 0.0f),
	    glm::vec3(0.0f, 0.0f, 1.0f),
	};

	const auto X = rot_ord[m_rot_order][0];
	const auto Y = rot_ord[m_rot_order][1];
	const auto Z = rot_ord[m_rot_order][2];

	auto matrix = glm::mat4(1.0f);

	switch (m_transform_type) {
		case 0: /* Pre Transform */
			matrix = pre_translate(matrix, m_pivot);
			matrix = pre_rotate(matrix, glm::radians(m_rotate[X]), axis[X]);
			matrix = pre_rotate(matrix, glm::radians(m_rotate[Y]), axis[Y]);
			matrix = pre_rotate(matrix, glm::radians(m_rotate[Z]), axis[Z]);
			matrix = pre_scale(matrix, scale);
			matrix = pre_translate(matrix, -m_pivot);
			matrix = pre_translate(matrix, m_translate);
			matrix = matrix * prim->matrix();
			break;
		case 1: /* Post Transform */
			matrix = post_translate(matrix, m_pivot);
			matrix = post_rotate(matrix, glm::radians(m_rotate[X]), axis[X]);
			matrix = post_rotate(matrix, glm::radians(m_rotate[Y]), axis[Y]);
			matrix = post_rotate(matrix, glm::radians(m_rotate[Z]), axis[Z]);
			matrix = post_scale(matrix, scale);
			matrix = post_translate(matrix, -m_pivot);
			matrix = post_translate(matrix, m_translate);
			matrix = prim->matrix() * matrix;
			break;
	}

	prim->matrix(matrix);

	setOutputPrimitive("Prim", prim);
}

void TransformNode::setUIParams(ParamCallback *cb)
{
	const char *xform_order_items[] = {
	    "Pre Transform", "Post Transform", nullptr
	};

	enum_param(cb, "Transform Order", &m_transform_type, xform_order_items, m_transform_type);

	const char *rot_order_items[] = {
	    "X Y Z",
	    "X Z Y",
	    "Y X Z",
	    "Y Z X",
	    "Z X Y",
	    "Z Y X",
	    nullptr
	};

	enum_param(cb, "Rotation Order", &m_rot_order, rot_order_items, m_rot_order);

	xyz_param(cb, "Translate", &m_translate[0]);
	xyz_param(cb, "Rotate", &m_rotate[0], 0.0f, 360.0f);
	xyz_param(cb, "Scale", &m_scale[0]);
	xyz_param(cb, "Pivot", &m_pivot[0]);

	float_param(cb, "Uniform Scale", &m_uniform_scale, 0.0f, 1000.0f, m_uniform_scale);

	bool_param(cb, "Invert Transformation", &m_invert, m_invert);
}

/* ************************************************************************** */

CreateBoxNode::CreateBoxNode()
    : Node("Box")
{
	addOutput("Prim");
}

void CreateBoxNode::process()
{
	Primitive *prim = new Mesh;
	auto mesh = static_cast<Mesh *>(prim);

	PointList *points = mesh->points();
	points->reserve(8);

	/* todo: expose this to the UI */
	const auto &x_div = 2;
	const auto &y_div = 2;
	const auto &z_div = 2;

	auto vec = glm::vec3{ 0.0f, 0.0f, 0.0f };

	const auto size = m_size * m_uniform_scale;

	const auto &start_x = -(size.x / 2.0f) + m_center.x;
	const auto &start_y = -(size.y / 2.0f) + m_center.y;
	const auto &start_z = -(size.z / 2.0f) + m_center.z;

	const auto &x_increment = size.x / (x_div - 1);
	const auto &y_increment = size.y / (y_div - 1);
	const auto &z_increment = size.z / (z_div - 1);

	for (auto x = 0; x < x_div; ++x) {
		vec[0] = start_x + x * x_increment;

		for (auto y = 0; y < y_div; ++y) {
			vec[1] = start_y + y * y_increment;

			for (auto z = 0; z < z_div; ++z) {
				vec[2] = start_z + z * z_increment;

				points->push_back(vec);
			}
		}
	}

	PolygonList *polys = mesh->polys();
	polys->resize(6);
	polys->push_back(glm::ivec4(1, 3, 2, 0));
	polys->push_back(glm::ivec4(3, 7, 6, 2));
	polys->push_back(glm::ivec4(7, 5, 4, 6));
	polys->push_back(glm::ivec4(5, 1, 0, 4));
	polys->push_back(glm::ivec4(0, 2, 6, 4));
	polys->push_back(glm::ivec4(5, 7, 3, 1));

	mesh->tagUpdate();

	setOutputPrimitive("Prim", prim);
}

void CreateBoxNode::setUIParams(ParamCallback *cb)
{
	xyz_param(cb, "Size", &m_size[0]);
	xyz_param(cb, "Center", &m_center[0]);

	float_param(cb, "Uniform Scale", &m_uniform_scale, 0.0f, 10.0f, m_uniform_scale);
}

/* ************************************************************************** */

CreateTorusNode::CreateTorusNode()
    : Node("Torus")
{
	addOutput("Prim");
}

void CreateTorusNode::process()
{
	Primitive *prim = new Mesh;
	auto mesh = static_cast<Mesh *>(prim);

	PointList *points = mesh->points();
	PolygonList *polys = mesh->polys();

	constexpr auto tau = static_cast<float>(M_PI) * 2.0f;

	const auto vertical_angle_stride = tau / static_cast<float>(m_major_segment);
	const auto horizontal_angle_stride = tau / static_cast<float>(m_minor_segment);

	int f1 = 0, f2, f3, f4;
	const auto tot_verts = m_major_segment * m_minor_segment;

	points->reserve(tot_verts);

	const auto &major_radius = m_major_radius * m_uniform_scale;
	const auto &minor_radius = m_minor_radius * m_uniform_scale;

	for (int i = 0; i < m_major_segment; ++i) {
		auto theta = vertical_angle_stride * i;

		for (int j = 0; j < m_minor_segment; ++j) {
			auto phi = horizontal_angle_stride * j;

			auto x = glm::cos(theta) * (major_radius + minor_radius * glm::cos(phi));
			auto y = minor_radius * glm::sin(phi);
			auto z = glm::sin(theta) * (major_radius + minor_radius * glm::cos(phi));

			points->push_back(glm::vec3(x, y, z) + m_center);

			if (j + 1 == m_minor_segment) {
				f2 = i * m_minor_segment;
				f3 = f1 + m_minor_segment;
				f4 = f2 + m_minor_segment;
			}
			else {
				f2 = f1 + 1;
				f3 = f1 + m_minor_segment;
				f4 = f3 + 1;
			}

			if (f2 >= tot_verts) {
				f2 -= tot_verts;
			}
			if (f3 >= tot_verts) {
				f3 -= tot_verts;
			}
			if (f4 >= tot_verts) {
				f4 -= tot_verts;
			}

			if (f2 > 0) {
				polys->push_back(glm::ivec4(f1, f3, f4, f2));
			}
			else {
				polys->push_back(glm::ivec4(f2, f1, f3, f4));
			}

			++f1;
		}
	}

	mesh->tagUpdate();

	setOutputPrimitive("Prim", prim);
}

void CreateTorusNode::setUIParams(ParamCallback *cb)
{
	xyz_param(cb, "Center", &m_center[0]);

	float_param(cb, "Major Radius", &m_major_radius, 0.0f, 10.0f, m_major_radius);
	float_param(cb, "Minor Radius", &m_minor_radius, 0.0f, 10.0f, m_minor_radius);

	int_param(cb, "Major Segment", &m_major_segment, 4, 100, m_major_segment);
	int_param(cb, "Minor Segment", &m_minor_segment, 4, 100, m_minor_segment);

	float_param(cb, "Uniform Scale", &m_uniform_scale, 0.0f, 10.0f, m_uniform_scale);
}

/* ************************************************************************** */

CreateGridNode::CreateGridNode()
    : Node("Grid")
{
	addOutput("Prim");
}

void CreateGridNode::process()
{
	Primitive *prim = new Mesh;
	auto mesh = static_cast<Mesh *>(prim);

	const auto totpoints = m_rows * m_columns;

	auto points = mesh->points();
	points->reserve(totpoints);

	auto vec = glm::vec3{ 0.0f, m_center.y, 0.0f };

	const auto &x_increment = m_size.x / (m_rows - 1);
	const auto &y_increment = m_size.y / (m_columns - 1);
	const auto &start_x = -(m_size.x / 2.0f) + m_center.x;
	const auto &start_y = -(m_size.y / 2.0f) + m_center.z;

	for (auto y = 0; y < m_columns; ++y) {
		vec[2] = start_y + y * y_increment;

		for (auto x = 0; x < m_rows; ++x) {
			vec[0] = start_x + x * x_increment;

			points->push_back(vec);
		}
	}

	PolygonList *polys = mesh->polys();

	auto quad = glm::ivec4{ 0, 0, 0, 0 };

	/* make a copy for the lambda */
	const auto xtot = m_rows;

	auto index = [&xtot](const int x, const int y)
	{
		return x + y * xtot;
	};

	for (auto y = 1; y < m_columns; ++y) {
		for (auto x = 1; x < m_rows; ++x) {
			quad[0] = index(x - 1, y - 1);
			quad[1] = index(x,     y - 1);
			quad[2] = index(x,     y    );
			quad[3] = index(x - 1, y    );

			polys->push_back(quad);
		}
	}

	mesh->tagUpdate();

	setOutputPrimitive("Prim", prim);
}

void CreateGridNode::setUIParams(ParamCallback *cb)
{
	xyz_param(cb, "Center", &m_center[0]);
	xyz_param(cb, "Size", &m_size[0]);

	int_param(cb, "Rows", &m_rows, 2, 100, m_rows);
	int_param(cb, "Columns", &m_columns, 2, 100, m_columns);
}

/* ************************************************************************** */

void register_builtin_nodes(NodeFactory *factory)
{
	REGISTER_NODE("Geometry", "Box", CreateBoxNode);
	REGISTER_NODE("Geometry", "Grid", CreateGridNode);
	REGISTER_NODE("Geometry", "Torus", CreateTorusNode);
	REGISTER_NODE("Geometry", "Transform", TransformNode);
}
