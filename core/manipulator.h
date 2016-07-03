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

#include <glm/gtx/rotate_vector.hpp>

#include <vector>

#include "transformable.h"
#include "util_render.h"

class RenderBuffer;
class ViewerContext;

class Manipulator : public Transformable {
	RenderBuffer *m_buffer[6];

	std::vector<glm::vec3> m_vertices;
	glm::vec3 m_dimensions;
	glm::vec3 m_min, m_max;

	bool m_first;

	glm::vec3 m_last_pos;
	glm::vec3 m_plane_pos;
	glm::vec3 m_plane_nor;
	glm::vec3 m_delta_pos;
	int m_axis;

public:
	Manipulator();
	~Manipulator();

	bool intersect(const Ray &ray, float &min);

	void render(const ViewerContext * const context, const bool for_outline);

	glm::vec3 update(const Ray &ray);

private:
	void applyConstraint(const glm::vec3 &cpos);
};
