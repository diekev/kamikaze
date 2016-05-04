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

#include "prim_points.h"

#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "context.h"

#ifndef UNUSED
#define UNUSED(x) static_cast<void>(x);
#endif

PrimPoints::PrimPoints()
{
	m_draw_type = GL_POINTS;
	loadShader();
}

PrimPoints::~PrimPoints()
{
	for (auto &attr : m_attributes) {
		delete attr;
	}
}

PointList *PrimPoints::points()
{
	return &m_points;
}

const PointList *PrimPoints::points() const
{
	return &m_points;
}

Attribute *PrimPoints::attribute(const std::string &name, AttributeType type)
{
	auto iter = std::find_if(m_attributes.begin(), m_attributes.end(),
	                         [&](Attribute *attr)
	{
		return (attr->type() == type) && (attr->name() == name);
	});

	if (iter == m_attributes.end()) {
		return nullptr;
	}

	return *iter;
}

void PrimPoints::addAttribute(Attribute *attr)
{
	if (attribute(attr->name(), attr->type()) == nullptr) {
		m_attributes.push_back(attr);
	}
}

Attribute *PrimPoints::addAttribute(const std::string &name, AttributeType type, size_t size)
{
	auto attr = attribute(name, type);

	if (attr == nullptr) {
		attr = new Attribute(name, type, size);
		m_attributes.push_back(attr);
	}

	return attr;
}

Primitive *PrimPoints::copy() const
{
	PrimPoints *prim = new PrimPoints;

	PointList *points = prim->points();
	points->resize(this->points()->size());

	prim->tagUpdate();
	return prim;
}

void PrimPoints::loadShader()
{
	m_program.load(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/flat_shader.vert"));
	m_program.load(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/flat_shader.frag"));
	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
		m_program.addUniform("for_outline");
	}
	m_program.disable();
}

void PrimPoints::render(ViewerContext *context, const bool for_outline)
{
	if (m_program.isValid()) {
		glPointSize(2.0f);

		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(m_matrix));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(context->MVP()));
		glUniform1i(m_program("for_outline"), for_outline);
		glDrawArrays(m_draw_type, 0, m_elements);

		m_buffer_data->unbind();
		m_program.disable();

		glPointSize(1.0f);
	}
}

void PrimPoints::setCustomUIParams(ParamCallback *cb)
{
	UNUSED(cb);
}

void PrimPoints::prepareRenderData()
{
	if (!m_need_data_update) {
		return;
	}

	computeBBox(m_min, m_max);

	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		m_points[i] = m_points[i] * glm::mat3(m_inv_matrix);
	}

	m_elements = this->points()->size();

	m_buffer_data.reset(new ego::BufferObject());
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(m_points.data(), m_points.byte_size());
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->unbind();

	ego::util::GPU_check_errors("Unable to create level set buffer");

	m_need_data_update = false;
}

void PrimPoints::computeBBox(glm::vec3 &min, glm::vec3 &max)
{
	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		auto vert = m_points[i];

		if (vert.x < m_min.x) {
			m_min.x = vert.x;
		}
		else if (vert.x > m_max.x) {
			m_max.x = vert.x;
		}

		if (vert.y < m_min.y) {
			m_min.y = vert.y;
		}
		else if (vert.y > m_max.y) {
			m_max.y = vert.y;
		}

		if (vert.z < m_min.z) {
			m_min.z = vert.z;
		}
		else if (vert.z > m_max.z) {
			m_max.z = vert.z;
		}
	}

	min = m_min;
	max = m_max;
	m_dimensions = m_max - m_min;
}
