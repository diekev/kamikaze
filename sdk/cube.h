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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#pragma once

#include <glm/glm.hpp>
#include <vector>

class RenderBuffer;
class ViewerContext;

class Cube {
	RenderBuffer *m_buffer;

	std::vector<glm::vec3> m_vertices;
	glm::vec3 m_dimensions, m_scale, m_rotation;
	glm::vec3 m_min, m_max, m_pos;
	glm::mat4 m_matrix, m_inv_matrix;

	void updateMatrix();

public:
	Cube(const glm::vec3 &min, const glm::vec3 &max);
	~Cube();

	/* Disallow copy. */
	Cube(const Cube &other) = delete;
	Cube &operator=(const Cube &other) = delete;

	void render(const ViewerContext * const context, const bool for_outline);
};
