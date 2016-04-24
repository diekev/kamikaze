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

#include "primitive.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include "paramfactory.h"

#include "ui/paramcallback.h"

#include "util/util_render.h"

bool Primitive::intersect(const Ray &ray, float &min) const
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

void Primitive::setDrawType(int draw_type)
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

void Primitive::drawBBox(const bool b)
{
	m_draw_bbox = b;
}

bool Primitive::drawBBox() const
{
	return m_draw_bbox;
}

Cube *Primitive::bbox() const
{
	return m_bbox.get();
}

glm::vec3 Primitive::pos() const
{
	return m_pos;
}

glm::vec3 &Primitive::pos()
{
	m_need_update = true;
	return m_pos;
}

glm::vec3 Primitive::scale() const
{
	return m_scale;
}

glm::vec3 &Primitive::scale()
{
	m_need_update = true;
	return m_scale;
}

glm::vec3 Primitive::rotation() const
{
	return m_rotation;
}

glm::vec3 &Primitive::rotation()
{
	m_need_update = true;
	return m_rotation;
}

void Primitive::flags(object_flags flags)
{
	m_flags = flags;
}

object_flags Primitive::flags() const
{
	return m_flags;
}

glm::mat4 Primitive::matrix() const
{
	return m_matrix;
}

glm::mat4 &Primitive::matrix()
{
	return m_matrix;
}

void Primitive::update()
{
	if (m_need_update) {
		updateMatrix();

		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

void Primitive::tagUpdate()
{
	m_need_update = true;
}

void Primitive::updateMatrix()
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

QString Primitive::name() const
{
	return m_name;
}

void Primitive::name(const QString &name)
{
	m_name = name;
}

void Primitive::setUIParams(ParamCallback *cb)
{
	string_param(cb, "Name", &m_name, "");

	bool_param(cb, "Draw BoundingBox", &m_draw_bbox, false);

	xyz_param(cb, "Position", &m_pos[0]);
	xyz_param(cb, "Scale", &m_scale[0]);
	xyz_param(cb, "Rotation", &m_rotation[0]);
}

int Primitive::refcount() const
{
	return m_refcount;
}

void Primitive::incref()
{
	++m_refcount;
}

void Primitive::decref()
{
	--m_refcount;
}

void PrimitiveFactory::registerType(const std::string &name, PrimitiveFactory::factory_func func)
{
	const auto iter = m_map.find(name);
	assert(iter == m_map.end());

	m_map[name] = func;
}

Primitive *PrimitiveFactory::operator()(const std::string &name)
{
	const auto iter = m_map.find(name);
	assert(iter != m_map.end());

	return iter->second();
}

size_t PrimitiveFactory::numEntries() const
{
	return m_map.size();
}

std::vector<std::string> PrimitiveFactory::keys() const
{
	std::vector<std::string> v;

	for (const auto &entry : m_map) {
		v.push_back(entry.first);
	}

	return v;
}

bool PrimitiveFactory::registered(const std::string &key) const
{
	return (m_map.find(key) != m_map.end());
}
