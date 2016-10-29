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

#include <algorithm>
#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "context.h"
#include "renderbuffer.h"

/* ************************************************************************** */

static RenderBuffer *create_point_buffer()
{
	RenderBuffer *renderbuffer = new RenderBuffer;

	renderbuffer->set_shader_source(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/flat_shader.vert"));
	renderbuffer->set_shader_source(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/flat_shader.frag"));
	renderbuffer->finalize_shader();

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_attribute("vertex_color");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("for_outline");
	params.add_uniform("color");
	params.add_uniform("has_vcolors");

	renderbuffer->set_shader_params(params);

	ego::Program *program = renderbuffer->program();
	program->uniform("color", 0.0f, 0.0f, 0.0f, 1.0f);

	DrawParams draw_params;
	draw_params.set_draw_type(GL_POINTS);
	draw_params.set_point_size(2.0f);

	renderbuffer->set_draw_params(draw_params);

	return renderbuffer;
}

/* ************************************************************************** */

size_t PrimPoints::id = -1;

PrimPoints::PrimPoints()
    : m_renderbuffer(nullptr)
{}

PrimPoints::PrimPoints(const PrimPoints &other)
    : Primitive(other)
    , m_renderbuffer(nullptr)
{
	/* Copy points. */
	auto points = other.points();
	m_points.resize(points->size());

	for (auto i = 0ul; i < points->size(); ++i) {
		m_points[i] = (*points)[i];
	}
}

PrimPoints::~PrimPoints()
{
	free_renderbuffer(m_renderbuffer);
}

PointList *PrimPoints::points()
{
	return &m_points;
}

const PointList *PrimPoints::points() const
{
	return &m_points;
}

Primitive *PrimPoints::copy() const
{
	auto prim = new PrimPoints(*this);
	prim->tagUpdate();

	return prim;
}

size_t PrimPoints::typeID() const
{
	return PrimPoints::id;
}

void PrimPoints::render(const ViewerContext &context)
{
	m_renderbuffer->render(context);
}

void PrimPoints::prepareRenderData()
{
	if (!m_need_data_update) {
		return;
	}

	if (!m_renderbuffer) {
		m_renderbuffer = create_point_buffer();
	}

	computeBBox(m_min, m_max);

	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		m_points[i] = m_points[i] * glm::mat3(m_inv_matrix);
	}

	m_renderbuffer->set_vertex_buffer("vertex",
	                                  m_points.data(),
	                                  m_points.byte_size(),
	                                  nullptr,
	                                  0,
	                                  m_points.size());

	auto colors = this->attribute("color", ATTR_TYPE_VEC3);

	if (colors != nullptr) {
		m_renderbuffer->set_color_buffer("vertex_color", colors->data(), colors->byte_size());
	}

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
