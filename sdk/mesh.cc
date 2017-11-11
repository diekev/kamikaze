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

#include "mesh.h"

#include <algorithm>
#include <ego/utils.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "outils/géométrie.h"
#include "outils/parallélisme.h"

#include "context.h"
#include "renderbuffer.h"

/* ************************************************************************** */

static RenderBuffer *create_surface_buffer()
{
	RenderBuffer *renderbuffer = new RenderBuffer;

	renderbuffer->set_shader_source(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/object.vert"));
	renderbuffer->set_shader_source(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/object.frag"));
	renderbuffer->finalize_shader();

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_attribute("normal");
	params.add_attribute("vertex_color");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("N");
	params.add_uniform("for_outline");
	params.add_uniform("color");
	params.add_uniform("has_vcolors");

	ego::Program *program = renderbuffer->program();
	program->uniform("color", 0.0f, 0.0f, 0.0f);

	renderbuffer->set_shader_params(params);

	return renderbuffer;
}

/* ************************************************************************** */

size_t Mesh::id = -1;

Mesh::Mesh()
    : Primitive()
    , m_renderbuffer(nullptr)
{
	add_attribute("normal", ATTR_TYPE_VEC3, 0);
	m_need_update = true;
}

Mesh::Mesh(const Mesh &other)
    : Primitive(other)
    , m_renderbuffer(nullptr)
{
	/* Copy points. */
	auto points = other.points();
	m_point_list.resize(points->size());

	for (auto i = 0ul; i < points->size(); ++i) {
		m_point_list[i] = (*points)[i];
	}

	/* Copy points. */
	auto polys = other.polys();
	m_poly_list.resize(polys->size());

	for (auto i = 0ul; i < polys->size(); ++i) {
		m_poly_list[i] = (*polys)[i];
	}
}

Mesh::~Mesh()
{
	free_renderbuffer(m_renderbuffer);
}

PointList *Mesh::points()
{
	return &m_point_list;
}

const PointList *Mesh::points() const
{
	return &m_point_list;
}

PolygonList *Mesh::polys()
{
	return &m_poly_list;
}

const PolygonList *Mesh::polys() const
{
	return &m_poly_list;
}

void Mesh::update()
{
	if (m_need_update) {
		computeBBox(m_min, m_max);

		m_bbox.reset(new Cube(m_min, m_max));
		m_need_update = false;
	}
}

Primitive *Mesh::copy() const
{
	auto mesh = new Mesh(*this);
	mesh->tagUpdate();

	return mesh;
}

size_t Mesh::typeID() const
{
	return Mesh::id;
}

void Mesh::render(const ViewerContext &context)
{
	/* Render vertices. */
	{
		DrawParams draw_params;
		draw_params.set_draw_type(GL_POINTS);
		draw_params.set_point_size(2.0f);

		m_renderbuffer->set_draw_params(draw_params);

		ego::Program *program = m_renderbuffer->program();
		program->enable();
		program->uniform("color", 0.0f, 0.0f, 0.0f);
		program->disable();

		m_renderbuffer->render(context);
	}

	/* Render surface. */
	{
		DrawParams draw_params;
		draw_params.set_draw_type(GL_TRIANGLES);

		m_renderbuffer->set_draw_params(draw_params);

		ego::Program *program = m_renderbuffer->program();
		program->enable();
		program->uniform("color", 1.0f, 1.0f, 1.0f);
		program->disable();

		m_renderbuffer->render(context);
	}
}

void Mesh::prepareRenderData()
{
	if (!m_need_data_update) {
		return;
	}

	if (!m_renderbuffer) {
		m_renderbuffer = create_surface_buffer();
	}

	for (size_t i = 0, ie = this->points()->size(); i < ie; ++i) {
		m_point_list[i] = m_point_list[i] * glm::mat3(m_inv_matrix);
	}

	auto indices = std::vector<unsigned int>{};
	indices.reserve(this->polys()->size());

	auto polys = this->polys();

	for (auto i = 0ul, ie = polys->size(); i < ie; ++i) {
		const auto &quad = (*polys)[i];

		indices.push_back(quad[0]);
		indices.push_back(quad[1]);
		indices.push_back(quad[2]);

		if (quad[3] != INVALID_INDEX) {
			indices.push_back(quad[0]);
			indices.push_back(quad[2]);
			indices.push_back(quad[3]);
		}
	}

	m_renderbuffer->can_outline(true);

	m_renderbuffer->set_vertex_buffer("vertex",
	                                  m_point_list.data(),
	                                  m_point_list.byte_size(),
	                                  &indices[0],
	                                  indices.size() * sizeof(GLuint),
	                                  indices.size());

	auto normals = this->attribute("normal", ATTR_TYPE_VEC3);

	if (normals != nullptr) {
		if (normals->size() != this->points()->size()) {
			auto normals = this->attribute("normal", ATTR_TYPE_VEC3);
			normals->resize(this->points()->size());

			auto polys = this->polys();

			calcule_normales(m_point_list, *polys, *normals, false);
		}

		m_renderbuffer->set_normal_buffer("normal", normals->data(), normals->byte_size());
	}

	auto colors = this->attribute("color", ATTR_TYPE_VEC3);

	if (colors != nullptr) {
		m_renderbuffer->set_color_buffer("vertex_color", colors->data(), colors->byte_size());
	}

	m_need_data_update = false;
}

void Mesh::computeBBox(glm::vec3 &min, glm::vec3 &max)
{
	calcule_boite_delimitation(m_point_list, m_min, m_max);

	min = m_min;
	max = m_max;
	m_dimensions = m_max - m_min;
}
