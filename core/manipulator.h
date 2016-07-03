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

class Transformable {
protected:
	glm::vec3 m_pos = glm::vec3(0.0f);
	glm::vec3 m_rot = glm::vec3(0.0f);
	glm::vec3 m_scale = glm::vec3(1.0f);

	glm::mat4 m_matrix = glm::mat4(1.0f);
	glm::mat4 m_inv_matrix = glm::mat4(0.0f);

public:
	Transformable() = default;
	virtual ~Transformable() = default;

	void update()
	{
		m_matrix = glm::mat4(1.0f);
		m_matrix = glm::translate(m_matrix, m_pos);
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_matrix = glm::rotate(m_matrix, glm::radians(m_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_matrix = glm::scale(m_matrix, m_scale);

		m_inv_matrix = glm::inverse(m_matrix);
	}

	void pos(const glm::vec3 &p)
	{
		m_pos = p;
		update();
	}

	const glm::vec3 &pos() const
	{
		return m_pos;
	}

	void rot(const glm::vec3 &r)
	{
		m_rot = r;
		update();
	}

	const glm::vec3 &rot() const
	{
		return m_rot;
	}

	void scale(const glm::vec3 &s)
	{
		m_scale = s;
		update();
	}

	const glm::vec3 &scale() const
	{
		return m_scale;
	}

	void matrix(const glm::mat4 &m)
	{
		m_matrix = m;
		m_inv_matrix = glm::inverse(m_matrix);
	}

	const glm::mat4 &matrix() const
	{
		return m_matrix;
	}
};

class Manipulator : public Transformable {
	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;
	unsigned int m_draw_type;

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

	bool intersect(const Ray &ray, float &min);

	void render(ViewerContext *context);

	glm::vec3 update(const Ray &ray);

private:
	void updateMatrix();
	void applyConstraint(const glm::vec3 &cpos);
};
