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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "object.h"

Object::Object()
	: m_buffer_data(nullptr)
    , m_draw_type(GL_QUADS)
    , m_dimensions(glm::vec3(0.0f))
    , m_scale(glm::vec3(1.0f))
    , m_inv_size(glm::vec3(0.0f))
    , m_rotation(glm::vec3(0.0f))
    , m_min(glm::vec3(0.0f))
    , m_max(glm::vec3(0.0f))
    , m_pos(glm::vec3(0.0f))
    , m_draw_bbox(false)
    , m_draw_topology(false)
    , m_need_update(false)
    , m_is_active(false)
{}

bool Object::intersect(const Ray &ray, float &min) const
{
	glm::vec3 inv_dir = 1.0f / ray.dir;
	glm::vec3 t_min = (m_min - ray.pos) * inv_dir;
	glm::vec3 t_max = (m_max - ray.pos) * inv_dir;
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

void Object::setDrawType(int draw_type)
{
	switch (draw_type) {
		case DRAW_WIRE:
			m_draw_type = GL_LINES;
			break;
		default:
		case DRAW_QUADS:
			m_draw_type = GL_QUADS;
			break;
	}
}

void Object::drawBBox(const bool b)
{
	m_draw_bbox = b;
}

void Object::drawTreeTopology(const bool b)
{
	m_draw_topology = b;
}

glm::vec3 Object::pos() const
{
	return m_pos;
}

void Object::setPos(const glm::vec3 &pos)
{
	m_pos = pos;
	m_need_update = true;
}

glm::vec3 Object::scale() const
{
	return m_scale;
}

void Object::setScale(const glm::vec3 &scale)
{
	m_scale = scale;
	m_need_update = true;
}

glm::vec3 Object::rotation() const
{
	return m_rotation;
}

void Object::setRotation(const glm::vec3 &rotation)
{
	m_rotation = rotation;
	m_need_update = true;
}

void Object::updateMatrix()
{
	m_min = m_pos - m_dimensions / 2.0f;
	m_max = m_min + m_dimensions;

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_pos);
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_matrix = glm::rotate(m_matrix, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_matrix = glm::scale(m_matrix, m_scale * m_dimensions);

	m_inv_matrix = glm::inverse(m_matrix);
}
