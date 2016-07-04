﻿/*
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

#include <kamikaze/context.h>

#include "renderbuffer.h"

#include "util/utils.h"
#include "util/utils_glm.h"

enum {
	AXIS_NONE = -1,

	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2,

	XY_PLANE = 3,
	XZ_PLANE = 4,
	YZ_PLANE = 5,
};

static bool intersect(const Ray &ray, const glm::vec3 &obmin, const glm::vec3 &obmax, float &min)
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

static bool intersect(const Ray &ray, const glm::vec3 &pos, const glm::vec3 &nor, glm::vec3 &ipos)
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

static constexpr auto EPSILON = 1e-6f;

/**
 * Möller-Trumbore intersection test.
 * From https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
static bool intersect(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const Ray &ray)
{
	/* Find vectors for two edges sharing v1. */
	const auto e1 = v2 - v1;
	const auto e2 = v3 - v1;

	/* Begin calculating determinant - also used to calculate u parameter. */
	const auto P = glm::cross(ray.dir, e2);

	/* If det is near zero, ray lies in plane of triangle or ray is parallel to
	 * plane of triangle. */
	const auto det = glm::dot(e1, P);

	if (det > -EPSILON && det < EPSILON) {
		return false;
	}

	const auto inv_det = 1.0f / det;

	/* Calculate distance from V1 to ray origin. */
	const auto T = ray.pos - v1;

	/* Calculate u parameter and test bound. */
	const auto u = glm::dot(T, P) * inv_det;

	/* The intersection lies outside of the triangle. */
	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	/* Prepare to test v parameter. */
	const auto Q = glm::cross(T, e1);

	/* Calculate V parameter and test bound. */
	const auto v = glm::dot(ray.dir, Q) * inv_det;

	/* The intersection lies outside of the triangle. */
	if (v < 0.0f || u + v  > 1.0f) {
		return false;
	}

	const auto t = glm::dot(e2, Q) * inv_det;

	if (t > EPSILON) {
		/* Ray intersection. */
		return true;
	}

	/* No hit, no win. */
	return false;
}

static bool intersect_quad(const glm::vec3 &v1, const glm::vec3 &v2,
                           const glm::vec3 &v3, const glm::vec3 &v4,
                           const Ray &ray)
{
	if (intersect(v1, v2, v3, ray)) {
		return true;
	}

	if (intersect(v1, v3, v4, ray)) {
		return true;
	}

	return false;
}

static void add_arrow(std::vector<glm::vec3> &points,
                      std::vector<unsigned int> &indices,
                      const int indices_offset,
                      const int axis)
{
	auto min = glm::vec3(0.0f, -0.01f, -0.01f);
	auto max = glm::vec3(2.0f, 0.01f, 0.01f);

	switch (axis) {
		default:
		case X_AXIS:
			/* Nothing to do. */
			break;
		case Y_AXIS:
			min = glm::rotateZ(min, static_cast<float>(M_PI_2));
			max = glm::rotateZ(max, static_cast<float>(M_PI_2));
			break;
		case Z_AXIS:
			min = glm::rotateY(min, static_cast<float>(M_PI_2));
			max = glm::rotateY(max, static_cast<float>(M_PI_2));
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

static void add_plane(std::vector<glm::vec3> &points,
                      std::vector<unsigned int> &indices,
                      const int indices_offset,
                      const int axis)
{
	glm::vec3 min, max;

	switch (axis) {
		default:
		case YZ_PLANE:
			min = glm::vec3(0.75f, 0.75f, 0.0f);
			max = glm::vec3(1.25f, 1.25f, 0.0f);

			points.push_back(glm::vec3(min[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], max[1], max[2]));
			points.push_back(glm::vec3(min[0], max[1], max[2]));
			break;
		case XZ_PLANE:
			min = glm::vec3(0.75f, 0.0f, -0.75f);
			max = glm::vec3(1.25f, 0.0f, -1.25f);

			points.push_back(glm::vec3(min[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], min[1], min[2]));
			points.push_back(glm::vec3(max[0], max[1], max[2]));
			points.push_back(glm::vec3(min[0], max[1], max[2]));
			break;
		case XY_PLANE:
			min = glm::vec3(0.0f, 0.75f, -0.75f);
			max = glm::vec3(0.0f, 1.25f, -1.25f);

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

static void add_cone(std::vector<glm::vec3> &points,
                     std::vector<unsigned int> &indices,
                     const int indices_offset,
                     const int axis)
{
	const auto &dia1 = 0.1f;
	const auto &dia2 = 0.0f;
	const auto &segs = 16;
	const auto &depth = 0.2f;

	const auto phid = 2.0f * static_cast<float>(M_PI) / segs;
	auto phi = 0.0f;

	auto rotate_vert = [&axis](const glm::vec3 &vec) -> glm::vec3
	{
		switch (axis) {
			case X_AXIS:
				return glm::rotateZ(vec, static_cast<float>(-M_PI_2));
			default:
			case Y_AXIS:
				/* Nothing to do. */
				return vec;
			case Z_AXIS:
				return glm::rotateX(vec, static_cast<float>(-M_PI_2));
		}
	};

	glm::vec3 vec(0.0f, 0.0f, 0.0f);

	const auto cent1 = 0;
	vec[1] = 2.0f - depth;

	vec = rotate_vert(vec);
	points.push_back(vec);

	const auto cent2 = 1;
	vec[0] = 0.0f;
	vec[1] = 2.0f + depth;
	vec[2] = 0.0f;

	vec = rotate_vert(vec);
	points.push_back(vec);

	auto firstv1 = 0;
	auto firstv2 = 0;
	auto lastv1 = 0;
	auto lastv2 = 0;
	auto v1 = 0;
	auto v2 = 0;

	auto index = 2;

	for (int a = 0; a < segs; ++a, phi += phid) {
		/* Going this way ends up with normal(s) upward */
		vec[0] = -dia1 * std::sin(phi);
		vec[1] = 2.0f - depth;
		vec[2] = dia1 * std::cos(phi);
		vec = rotate_vert(vec);

		v1 = index++;
		points.push_back(vec);

		vec[0] = -dia2 * std::sin(phi);
		vec[1] = 2.0f + depth;
		vec[2] = dia2 * std::cos(phi);
		vec = rotate_vert(vec);

		v2 = index++;
		points.push_back(vec);

		if (a > 0) {
			/* Poly for the bottom cap. */
			indices.push_back(cent1 + indices_offset);
			indices.push_back(lastv1 + indices_offset);
			indices.push_back(v1 + indices_offset);

			/* Poly for the top cap. */
			indices.push_back(cent2 + indices_offset);
			indices.push_back(v2 + indices_offset);
			indices.push_back(lastv2 + indices_offset);

			/* Poly for the side. */
			indices.push_back(lastv1 + indices_offset);
			indices.push_back(lastv2 + indices_offset);
			indices.push_back(v2 + indices_offset);
			indices.push_back(lastv1 + indices_offset);
			indices.push_back(lastv2 + indices_offset);
			indices.push_back(v1 + indices_offset);
		}
		else {
			firstv1 = v1;
			firstv2 = v2;
		}

		lastv1 = v1;
		lastv2 = v2;
	}

	/* Poly for the bottom cap. */
	indices.push_back(cent1 + indices_offset);
	indices.push_back(v1 + indices_offset);
	indices.push_back(firstv1 + indices_offset);

	/* Poly for the top cap. */
	indices.push_back(cent2 + indices_offset);
	indices.push_back(firstv2 + indices_offset);
	indices.push_back(v2 + indices_offset);

	/* Poly for the side. */
	indices.push_back(v1 + indices_offset);
	indices.push_back(v2 + indices_offset);
	indices.push_back(firstv2 + indices_offset);
	indices.push_back(v1 + indices_offset);
	indices.push_back(firstv2 + indices_offset);
	indices.push_back(firstv1 + indices_offset);
}

static RenderBuffer *add_arrow_buffer(const int axis)
{
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	add_arrow(vertices, indices, vertices.size(), axis);
	add_cone(vertices, indices, vertices.size(), axis);

	RenderBuffer *buffer = new RenderBuffer;

	/* Setup shader. */
	buffer->set_shader_source(ego::VERTEX_SHADER,
	                          ego::util::str_from_file("shaders/tree_topology.vert"));
	buffer->set_shader_source(ego::FRAGMENT_SHADER,
	                          ego::util::str_from_file("shaders/tree_topology.frag"));

	buffer->finalize_shader();
	buffer->can_outline(true);

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_attribute("color");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("for_outline");

	buffer->set_shader_params(params);

	/* Setup vertices buffers. */
	std::vector<glm::vec3> colors(vertices.size());
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	color[axis] = 1.0f;

	std::fill(colors.begin(), colors.end(), color);

	buffer->set_vertex_buffer("vertex", vertices, indices);
	buffer->set_extra_buffer("color", colors);

	return buffer;
}

static RenderBuffer *add_plane_buffer(const int plane)
{
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	add_plane(vertices, indices, vertices.size(), plane);

	RenderBuffer *buffer = new RenderBuffer;

	/* Setup shader. */
	buffer->set_shader_source(ego::VERTEX_SHADER,
	                          ego::util::str_from_file("shaders/tree_topology.vert"));
	buffer->set_shader_source(ego::FRAGMENT_SHADER,
	                          ego::util::str_from_file("shaders/tree_topology.frag"));

	buffer->finalize_shader();
	buffer->can_outline(true);

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_attribute("color");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("for_outline");

	buffer->set_shader_params(params);

	/* Setup vertices buffers. */
	std::vector<glm::vec3> colors(vertices.size());
	glm::vec3 color(0.0f, 0.0f, 0.0f);
	color[plane - XY_PLANE] = 1.0f;

	std::fill(colors.begin(), colors.end(), color);

	buffer->set_vertex_buffer("vertex", vertices, indices);
	buffer->set_extra_buffer("color", colors);

	return buffer;
}

Manipulator::Manipulator()
    : Transformable()
{
	m_min = glm::vec3(0.0f, 0.0f, 0.0f);
	m_max = glm::vec3(1.0f, 1.0f, 1.0f);
	m_dimensions = m_max - m_min;

	m_last_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_delta_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_plane_pos = glm::vec3(0.0f, 0.0f, 0.0f);

	recompute_matrix();

	m_buffer[X_AXIS] = add_arrow_buffer(X_AXIS);
	m_buffer[Y_AXIS] = add_arrow_buffer(Y_AXIS);
	m_buffer[Z_AXIS] = add_arrow_buffer(Z_AXIS);

	m_buffer[XY_PLANE] = add_plane_buffer(XY_PLANE);
	m_buffer[XZ_PLANE] = add_plane_buffer(XZ_PLANE);
	m_buffer[YZ_PLANE] = add_plane_buffer(YZ_PLANE);
}

Manipulator::~Manipulator()
{
	delete m_buffer[X_AXIS];
	delete m_buffer[Y_AXIS];
	delete m_buffer[Z_AXIS];

	delete m_buffer[XY_PLANE];
	delete m_buffer[XZ_PLANE];
	delete m_buffer[YZ_PLANE];
}

bool Manipulator::intersect(const Ray &ray, float &min)
{
	m_last_pos = pos();
	m_first = true;

	/* Check X-axis. */
	auto nor = ray.pos - ray.dir;
	auto xmin = glm::vec3{ 0.0f, -0.05f, -0.05f }, xmax = glm::vec3{ 2.0f, 0.05f, 0.05f };
	xmin = xmin * glm::mat3(matrix()) + pos();
	xmax = xmax * glm::mat3(matrix()) + pos();

	if (::intersect(ray, xmin, xmax, min)) {
		m_axis = X_AXIS;
		m_plane_nor = glm::vec3{ 0.0f, nor.y, nor.z };
		return true;
	}

	/* Check Y-axis. */
	auto ymin = glm::vec3{ -0.05f, 0.0f, -0.05f }, ymax = glm::vec3{ 0.05f, 2.0f, 0.05f };
	ymin = ymin * glm::mat3(matrix()) + pos();
	ymax = ymax * glm::mat3(matrix()) + pos();

	if (::intersect(ray, ymin, ymax, min)) {
		m_axis = Y_AXIS;
		m_plane_nor = glm::vec3{ nor.x, 0.0f, nor.z };
		return true;
	}

	/* Check Z-axis. */
	auto zmin = glm::vec3{ -0.05f, -0.05f, -2.0f }, zmax = glm::vec3{ 0.05f, 0.05f, 0.0f };
	zmin = zmin * glm::mat3(matrix()) + pos();
	zmax = zmax * glm::mat3(matrix()) + pos();

	if (::intersect(ray, zmin, zmax, min)) {
		m_axis = Z_AXIS;
		m_plane_nor = glm::vec3{ nor.x, nor.y, 0.0f };
		return true;
	}

	{
		const auto &min = glm::vec3(0.75f, 0.75f, 0.0f) + pos();
		const auto &max = glm::vec3(1.25f, 1.25f, 0.0f) + pos();

		const auto &v1 = glm::vec3(min[0], min[1], min[2]);
		const auto &v2 = glm::vec3(max[0], min[1], min[2]);
		const auto &v3 = glm::vec3(max[0], max[1], max[2]);
		const auto &v4 = glm::vec3(min[0], max[1], max[2]);

		if (intersect_quad(v1, v2, v3, v4, ray)) {
			m_axis = YZ_PLANE;
			m_plane_nor =  glm::vec3{ 0.0f, 0.0f, 1.0f };
			m_plane_pos =  glm::vec3{ 1.0f, 1.0f, 0.0f };
			return true;
		}
	}

	{
		const auto &min = glm::vec3(0.75f, 0.0f, -0.75f) + pos();
		const auto &max = glm::vec3(1.25f, 0.0f, -1.25f) + pos();

		const auto &v1 = glm::vec3(min[0], min[1], min[2]);
		const auto &v2 = glm::vec3(max[0], min[1], min[2]);
		const auto &v3 = glm::vec3(max[0], max[1], max[2]);
		const auto &v4 = glm::vec3(min[0], max[1], max[2]);

		if (intersect_quad(v1, v2, v3, v4, ray)) {
			m_axis = XZ_PLANE;
			m_plane_nor =  glm::vec3{ 0.0f, 1.0f, 0.0f };
			m_plane_pos =  glm::vec3{ 1.0f, 0.0f, 1.0f };
			return true;
		}
	}

	{
		const auto &min = glm::vec3(0.0f, 0.75f, -0.75f) + pos();
		const auto &max = glm::vec3(0.0f, 1.25f, -1.25f) + pos();

		const auto &v1 = glm::vec3(min[0], min[1], min[2]);
		const auto &v2 = glm::vec3(max[0], min[1], max[2]);
		const auto &v3 = glm::vec3(max[0], max[1], max[2]);
		const auto &v4 = glm::vec3(min[0], max[1], min[2]);

		if (intersect_quad(v1, v2, v3, v4, ray)) {
			m_axis = XY_PLANE;
			m_plane_nor =  glm::vec3{ 1.0f, 0.0f, 0.0f };
			m_plane_pos =  glm::vec3{ 0.0f, 1.0f, 1.0f };
			return true;
		}
	}

	m_axis = AXIS_NONE;
	return false;
}

glm::vec3 Manipulator::update(const Ray &ray)
{
	if (m_axis < X_AXIS || m_axis > YZ_PLANE) {
		return this->pos();
	}

	/* find intersectiopn between ray and plane */
	glm::vec3 ipos;
	if (::intersect(ray, m_plane_pos * pos(), m_plane_nor, ipos)) {
		if (m_first) {
			m_delta_pos = ipos - m_last_pos;
			m_first = false;
		}

		this->applyConstraint(ipos - m_delta_pos);

		m_plane_pos = this->pos();
		m_last_pos = this->pos();

		this->recompute_matrix();
	}

	return this->pos();
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

void Manipulator::render(const ViewerContext * const context, const bool for_outline)
{
	if (for_outline && m_axis != AXIS_NONE) {
		m_buffer[m_axis]->render(context, for_outline);
		return;
	}

	m_buffer[X_AXIS]->render(context, for_outline);
	m_buffer[Y_AXIS]->render(context, for_outline);
	m_buffer[Z_AXIS]->render(context, for_outline);
	m_buffer[XY_PLANE]->render(context, for_outline);
	m_buffer[XZ_PLANE]->render(context, for_outline);
	m_buffer[YZ_PLANE]->render(context, for_outline);
}
