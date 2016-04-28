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

#include <ego/bufferobject.h>
#include <ego/program.h>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

#include "util_render.h"

class ViewerContext;

class Manipulator {
	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;
	unsigned int m_draw_type;

	std::vector<glm::vec3> m_vertices;
	glm::vec3 m_dimensions, m_scale, m_rotation;
	glm::vec3 m_min, m_max, m_pos;
	glm::mat4 m_matrix, m_inv_matrix;

	bool m_draw_bbox;

	glm::vec3 m_last_pos;
	glm::vec3 m_plane_pos;
	glm::vec3 m_plane_nor;
	glm::vec3 m_delta_pos;
	int m_axis;

public:
	Manipulator();

	bool intersect(const Ray &ray, float &min);

	void render(ViewerContext *context);

	void pos(const glm::vec3 &p);
	const glm::vec3 &pos() const;

	void update(const Ray &ray);

private:
	void updateMatrix();
	void applyConstraint(const glm::vec3 &cpos);
};
