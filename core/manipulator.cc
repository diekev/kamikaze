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

#include "manipulator.h"

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <kamikaze/context.h>
#include "util/utils.h"
#include "util/utils_glm.h"

enum {
	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2,

	XY_PLANE = 3,
	XZ_PLANE = 4,
	YZ_PLANE = 5,
};

bool intersect(const Ray &ray, const glm::vec3 &obmin, const glm::vec3 &obmax, float &min)
{
	glm::vec3 inv_dir = 1.0f / ray.dir;
	glm::vec3 t_min = (obmin - ray.pos) * inv_dir;
	glm::vec3 t_max = (obmax - ray.pos) * inv_dir;
	glm::vec3 t1 = glm::min(t_min, t_max);
	glm::vec3 t2 = glm::max(t_min, t_max);
	float t_near = glm::max(t1.x, glm::max(t1.y, t1.z));
	float t_far = glm::min(t2.x, glm::min(t2.y, t2.z));

	if (t_near < t_far && t_near < min) {
		min = t_near;
		return true;
	}

	return false;
}

bool intersect(const Ray &ray, const glm::vec3 &pos, const glm::vec3 &nor, glm::vec3 &ipos)
{
	const auto &u = ray.dir - ray.pos;
	const auto &w = ray.pos - pos;

	const auto &D = glm::dot(nor, u);
	const auto &N = -glm::dot(nor, w);

	/* segment is parallel to the plane */
	if (glm::abs(D) < 1e-6f) {
		/* segment lies on the plane */
		if (N == 0) {
			return false;
		}

		return false;
	}

	const auto &s = N / D;

	if (s < 0.0f || s > 1.0f) {
		return false;
	}

	ipos = ray.pos + u * s;
	return true;
}

void add_arrow(std::vector<glm::vec3> &points,
               std::vector<unsigned int> &indices,
               const int indices_offset,
               const int axis)
{
	auto min = glm::vec3(0.0f, -0.01f, -0.01f);
	auto max = glm::vec3(2.0f, 0.01f, 0.01f);

	switch (axis) {
		default:
		case 0:
			/* Nothing to do. */
			break;
		case 1:
			min = glm::rotateY(min, static_cast<float>(M_PI_2));
			max = glm::rotateY(max, static_cast<float>(M_PI_2));
			break;
		case 2:
			min = glm::rotateZ(min, static_cast<float>(M_PI_2));
			max = glm::rotateZ(max, static_cast<float>(M_PI_2));
			break;
	}

	/* cuboid */
	points.push_back(glm::vec3(min[0], min[1], min[2]));
	points.push_back(glm::vec3(max[0], min[1], min[2]));
	points.push_back(glm::vec3(max[0], max[1], min[2]));
	points.push_back(glm::vec3(min[0], max[1], min[2]));
	points.push_back(glm::vec3(min[0], min[1], max[2]));
	points.push_back(glm::vec3(max[0], min[1], max[2]));
	points.push_back(glm::vec3(max[0], max[1], max[2]));
	points.push_back(glm::vec3(min[0], max[1], max[2]));

	/* arrowhead */
	/* TODO */

	unsigned int indexes[24] = {
	    1, 0, 4, 5,
	    2, 1, 5, 6,
	    3, 2, 6, 7,
	    0, 3, 7, 4,
	    2, 3, 0, 1,
	    5, 4, 7, 6,
	};

	for (int i = 0; i < 6; ++i) {
		indices.push_back(indexes[4 * i] + indices_offset);
		indices.push_back(indexes[4 * i + 1] + indices_offset);
		indices.push_back(indexes[4 * i + 2] + indices_offset);
		indices.push_back(indexes[4 * i] + indices_offset);
		indices.push_back(indexes[4 * i + 2] + indices_offset);
		indices.push_back(indexes[4 * i + 3] + indices_offset);
	}
}

void add_plane(std::vector<glm::vec3> &points,
               std::vector<unsigned int> &indices,
               const int indices_offset,
               const int axis)
{
	glm::vec3 min, max;

	switch (axis) {
		default:
		case XY_PLANE:
			min = glm::vec3(0.5f, 0.5f, 0.0f);
			max = glm::vec3(1.5f, 1.5f, 0.0f);

			points.push_back(glm::vec3(min[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], max[1], max[2]));
			points.push_back(glm::vec3(min[0], max[1], max[2]));
			break;
		case XZ_PLANE:
			min = glm::vec3(0.5f, 0.0f, -0.5f);
			max = glm::vec3(1.5f, 0.0f, -1.5f);

			points.push_back(glm::vec3(min[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], max[1], max[2]));
			points.push_back(glm::vec3(min[0], max[1], max[2]));
			break;
		case YZ_PLANE:
			min = glm::vec3(0.0f, 0.5f, -0.5f);
			max = glm::vec3(0.0f, 1.5f, -1.5f);

			/* This one is slightly different than the other two, need to find a
			 * better order to add points to avoid this kind of duplication. */
			points.push_back(glm::vec3(min[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], min[1], max[2]));
			points.push_back(glm::vec3(max[0], max[1], max[2]));
			points.push_back(glm::vec3(min[0], max[1], min[2]));
			break;
	}

	indices.push_back(0 + indices_offset);
	indices.push_back(1 + indices_offset);
	indices.push_back(2 + indices_offset);
	indices.push_back(0 + indices_offset);
	indices.push_back(2 + indices_offset);
	indices.push_back(3 + indices_offset);
}

void Manipulator::updateMatrix()
{
	Transformable::update();
	std::cerr << "Manipulator pos: " << pos() << '\n';
	std::cerr << "Manipulator size: " << scale() << '\n';
	std::cerr << "Manipulator rot: " << rot() << '\n';
}

Manipulator::Manipulator()
    : Transformable()
{
	m_program.load(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/tree_topology.vert"));
	m_program.load(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/tree_topology.frag"));

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addAttribute("color");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
	}
	m_program.disable();

	m_draw_type = GL_TRIANGLES;

	m_min = glm::vec3(0.0f, 0.0f, 0.0f);
	m_max = glm::vec3(1.0f, 1.0f, 1.0f);
	m_dimensions = m_max - m_min;

	m_last_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_delta_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_plane_pos = glm::vec3(0.0f, 0.0f, 0.0f);

	updateMatrix();

	std::vector<unsigned int> indices;

	/* generate vertices */
	add_arrow(m_vertices, indices, m_vertices.size(), X_AXIS);
	add_plane(m_vertices, indices, m_vertices.size(), YZ_PLANE);
	add_arrow(m_vertices, indices, m_vertices.size(), Y_AXIS);
	add_plane(m_vertices, indices, m_vertices.size(), XZ_PLANE);
	add_arrow(m_vertices, indices, m_vertices.size(), Z_AXIS);
	add_plane(m_vertices, indices, m_vertices.size(), XY_PLANE);

	m_elements = indices.size();

	for (auto &vert : m_vertices) {
		vert = vert * glm::mat3(m_inv_matrix);
	}

	/* generate colors */
	std::vector<glm::vec3> colors;
	colors.resize(m_vertices.size());

	const auto stride = m_vertices.size() / 3;
	for (int i = 0; i < stride; ++i) {
		colors[i]              = glm::vec3(1.0f, 0.0f, 0.0f);
		colors[i + stride]     = glm::vec3(0.0f, 1.0f, 0.0f);
		colors[i + stride * 2] = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	/* generate buffer */
	m_buffer_data = ego::BufferObject::create();
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&m_vertices[0][0], m_vertices.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&indices[0], indices.size() * sizeof(int));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->generateNormalBuffer(&colors[0][0], m_vertices.size() * sizeof(glm::vec3));
	m_buffer_data->attribPointer(m_program["color"], 3);
	m_buffer_data->unbind();
}

bool Manipulator::intersect(const Ray &ray, float &min)
{
	/* Check X-axis. */
	auto nor = ray.pos - ray.dir;
	auto xmin = glm::vec3{ -1.0f, -0.05f, -0.05f }, xmax = glm::vec3{ 1.0f, 0.05f, 0.05f };
	if (::intersect(ray, xmin * glm::mat3(matrix()), xmax * glm::mat3(matrix()), min)) {
		m_axis = X_AXIS;
		m_plane_nor = glm::vec3{ 0.0f, nor.y, nor.z };
		return true;
	}

	/* Check Y-axis. */
	auto ymin = glm::vec3{ -0.05f, -1.0f, -0.05f }, ymax = glm::vec3{ 0.05f, 1.0f, 0.05f };
	if (::intersect(ray, ymin * glm::mat3(matrix()), ymax * glm::mat3(matrix()), min)) {
		m_axis = Y_AXIS;
		m_plane_nor = glm::vec3{ nor.x, 0.0f, nor.z };
		return true;
	}

	/* Check Z-axis. */
	auto zmin = glm::vec3{ -0.05f, -0.05f, -1.0f }, zmax = glm::vec3{ 0.05f, 0.05f, 1.0f };
	if (::intersect(ray, zmin * glm::mat3(matrix()), zmax * glm::mat3(matrix()), min)) {
		m_axis = Z_AXIS;
		m_plane_nor = glm::vec3{ nor.x, nor.y, 0.0f };
		return true;
	}

	/* Check XY-Plane */
	glm::vec3 dummy;
	if (::intersect(ray, glm::vec3{ 1.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, dummy)) {
		m_axis = XY_PLANE;
		m_plane_nor =  glm::vec3{ 0.0f, 0.0f, -1.0f };
		m_plane_pos =  glm::vec3{ 1.0f, 1.0f, 0.0f };
		std::cerr << "Isect XY Plane\n";
		return true;
	}

	/* Check XZ-Plane */
	if (::intersect(ray, glm::vec3{ 1.0f, 0.0f, -1.0f }, glm::vec3{ 0.0f, -1.0f, 0.0f }, dummy)) {
		m_axis = XZ_PLANE;
		m_plane_nor =  glm::vec3{ 0.0f, 1.0f, 0.0f };
		m_plane_pos =  glm::vec3{ 1.0f, 0.0f, -1.0f };
		std::cerr << "Isect XZ Plane\n";
		return true;
	}

	/* Check YZ-Plane */
	if (::intersect(ray, glm::vec3{ 0.0f, 1.0f, -1.0f }, glm::vec3{ 1.0f, 0.0f, 0.0f }, dummy)) {
		m_axis = YZ_PLANE;
		m_plane_nor =  glm::vec3{ 1.0f, 0.0f, 0.0f };
		m_plane_pos =  glm::vec3{ 0.0f, 1.0f, -1.0f };
		std::cerr << "Isect YZ Plane\n";
		return true;
	}

	m_axis = -1;
	return false;
}

void Manipulator::update(const Ray &ray)
{
	if (m_axis < X_AXIS || m_axis > YZ_PLANE) {
		return;
	}

	/* find intersectiopn between ray and plane */
	glm::vec3 ipos;
	if (::intersect(ray, m_plane_pos, m_plane_nor, ipos)) {
		applyConstraint(ipos);

		m_plane_pos = pos();
		m_delta_pos = pos() - m_last_pos;
		m_last_pos = pos();

		updateMatrix();
	}
}

void Manipulator::applyConstraint(const glm::vec3 &cpos)
{
	switch (m_axis) {
		case X_AXIS:
		case Y_AXIS:
		case Z_AXIS:
			m_pos[m_axis] = cpos[m_axis];
			break;
		case XY_PLANE:
		case XZ_PLANE:
		case YZ_PLANE:
			pos(cpos);
			break;
	}
}

void Manipulator::render(ViewerContext *context)
{
	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(matrix()));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));
		glDrawElements(m_draw_type, m_elements, GL_UNSIGNED_INT, nullptr);

		m_buffer_data->unbind();
		m_program.disable();
	}
}
