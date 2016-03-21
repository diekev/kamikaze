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

#include "object.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "modifiers.h"
#include "paramfactory.h"

#include "ui/paramcallback.h"

#include "util/util_render.h"

Object::Object()
    : m_draw_type(GL_TRIANGLES)
    , m_dimensions(glm::vec3(0.0f))
    , m_scale(glm::vec3(1.0f))
    , m_inv_size(glm::vec3(0.0f))
    , m_rotation(glm::vec3(0.0f))
    , m_min(glm::vec3(0.0f))
    , m_max(glm::vec3(0.0f))
    , m_pos(glm::vec3(0.0f))
    , m_name("")
    , m_draw_bbox(false)
    , m_need_update(true)
    , m_flags(object_flags::object_flags_none)
{}

Object::~Object()
{
	for (auto &modifier : m_modifiers) {
		delete modifier;
	}
}

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
		case DRAW_SOLID:
			m_draw_type = GL_TRIANGLES;
			break;
	}
}

void Object::drawBBox(const bool b)
{
	m_draw_bbox = b;
}

bool Object::drawBBox() const
{
	return m_draw_bbox;
}

Cube *Object::bbox() const
{
	return m_bbox.get();
}

glm::vec3 Object::pos() const
{
	return m_pos;
}

glm::vec3 &Object::pos()
{
	m_need_update = true;
	return m_pos;
}

glm::vec3 Object::scale() const
{
	return m_scale;
}

glm::vec3 &Object::scale()
{
	m_need_update = true;
	return m_scale;
}

glm::vec3 Object::rotation() const
{
	return m_rotation;
}

glm::vec3 &Object::rotation()
{
	m_need_update = true;
	return m_rotation;
}

void Object::flags(object_flags flags)
{
	m_flags = flags;
}

object_flags Object::flags() const
{
	return m_flags;
}

glm::mat4 Object::matrix() const
{
	return m_matrix;
}

glm::mat4 &Object::matrix()
{
	return m_matrix;
}

void Object::update()
{
	if (m_need_update) {
		updateMatrix();

		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

void Object::tagUpdate()
{
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

QString Object::name() const
{
	return m_name;
}

void Object::name(const QString &name)
{
	m_name = name;
}

void Object::addModifier(Modifier *modifier)
{
	m_modifiers.push_back(modifier);
}

std::vector<Modifier *> Object::modifiers() const
{
	return m_modifiers;
}

void Object::setUIParams(ParamCallback *cb)
{
	string_param(cb, "Name", &m_name, "");

	bool_param(cb, "Draw BoundingBox", &m_draw_bbox, false);

	xyz_param(cb, "Position", &m_pos[0]);
	xyz_param(cb, "Scale", &m_scale[0]);
	xyz_param(cb, "Rotation", &m_rotation[0]);
}
