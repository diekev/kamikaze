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

#include "segmentprim.h"

#include <algorithm>
#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "outils/géométrie.h"

#include "context.h"
#include "renderbuffer.h"

/* ************************************************************************** */

static RenderBuffer *create_point_buffer()
{
	RenderBuffer *renderbuffer = new RenderBuffer;

	renderbuffer->set_shader_source(numero7::ego::VERTEX_SHADER, numero7::ego::util::str_from_file("shaders/flat_shader.vert"));
	renderbuffer->set_shader_source(numero7::ego::FRAGMENT_SHADER, numero7::ego::util::str_from_file("shaders/flat_shader.frag"));
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

	numero7::ego::Program *program = renderbuffer->program();
	program->uniform("color", 0.0f, 0.0f, 0.0f);

	DrawParams draw_params;
	draw_params.set_draw_type(GL_LINES);

	renderbuffer->set_draw_params(draw_params);

	return renderbuffer;
}

/* ************************************************************************** */

size_t SegmentPrim::id = -1;

SegmentPrim::SegmentPrim()
    : m_renderbuffer(nullptr)
{}

SegmentPrim::SegmentPrim(const SegmentPrim &other)
    : Primitive(other)
    , m_renderbuffer(nullptr)
{
	/* Copy points. */
	auto points = other.points();
	m_points.resize(points->size());

	for (auto i = 0ul; i < points->size(); ++i) {
		m_points[i] = (*points)[i];
	}

	/* Copy edges. */
	auto edges = other.edges();
	m_edges.resize(edges->size());

	for (auto i = 0ul; i < edges->size(); ++i) {
		m_edges[i] = (*edges)[i];
	}
}

SegmentPrim::~SegmentPrim()
{
	free_renderbuffer(m_renderbuffer);
}

PointList *SegmentPrim::points()
{
	return &m_points;
}

const PointList *SegmentPrim::points() const
{
	return &m_points;
}

EdgeList *SegmentPrim::edges()
{
	return &m_edges;
}

const EdgeList *SegmentPrim::edges() const
{
	return &m_edges;
}

size_t SegmentPrim::nombre_courbes() const
{
	m_nombre_courbes;
}

void SegmentPrim::nombre_courbes(size_t nombre)
{
	m_nombre_courbes = nombre;
}

size_t SegmentPrim::points_par_courbe() const
{
	return m_points_par_courbe;
}

void SegmentPrim::points_par_courbe(size_t nombre)
{
	m_points_par_courbe = nombre;
}

Primitive *SegmentPrim::copy() const
{
	auto prim = new SegmentPrim(*this);
	prim->tagUpdate();

	return prim;
}

size_t SegmentPrim::typeID() const
{
	return SegmentPrim::id;
}

void SegmentPrim::render(const ViewerContext &context)
{
	/* Render vertices. */
	{
		DrawParams draw_params;
		draw_params.set_draw_type(GL_POINTS);
		draw_params.set_point_size(2.0f);

		m_renderbuffer->set_draw_params(draw_params);
		m_renderbuffer->render(context);
	}

	/* Render surface. */
	{
		DrawParams draw_params;
		draw_params.set_draw_type(GL_LINES);

		m_renderbuffer->set_draw_params(draw_params);
		m_renderbuffer->render(context);
	}
}

void SegmentPrim::prepareRenderData()
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

	auto edgelist = this->edges();
	auto indices = std::vector<unsigned int>{};
	indices.reserve(edgelist->size());

	for (auto i = 0ul, ie = edgelist->size(); i < ie; ++i) {
		const auto &edge = (*edgelist)[i];
		indices.push_back(edge[0]);
		indices.push_back(edge[1]);
	}

	m_renderbuffer->set_vertex_buffer("vertex",
	                                  m_points.data(),
	                                  m_points.byte_size(),
	                                  &indices[0],
	                                  indices.size() * sizeof(GLuint),
	                                  indices.size());

	auto colors = this->attribute("color", ATTR_TYPE_VEC3);

	if (colors != nullptr) {
		m_renderbuffer->set_color_buffer("vertex_color", colors->data(), colors->byte_size());
	}

	m_need_data_update = false;
}

void SegmentPrim::computeBBox(glm::vec3 &min, glm::vec3 &max)
{
	calcule_boite_delimitation(m_points, m_min, m_max);

	min = m_min;
	max = m_max;
	m_dimensions = m_max - m_min;
}
