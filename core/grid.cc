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

#include "grid.h"

#include <algorithm>
#include <ego/utils.h>
#include <GL/glew.h>

#include <kamikaze/renderbuffer.h>

static RenderBuffer *create_buffer(const glm::vec4 &color, float line_size)
{
	RenderBuffer *buffer = new RenderBuffer;

	buffer->set_shader_source(ego::VERTEX_SHADER, ego::util::str_from_file("shaders/flat_shader.vert"));
	buffer->set_shader_source(ego::FRAGMENT_SHADER, ego::util::str_from_file("shaders/flat_shader.frag"));
	buffer->finalize_shader();

	ProgramParams params;
	params.add_attribute("vertex");
	params.add_uniform("matrix");
	params.add_uniform("MVP");
	params.add_uniform("color");

	buffer->set_shader_params(params);

	DrawParams draw_params;
	draw_params.set_draw_type(GL_LINES);
	draw_params.set_line_size(line_size);

	buffer->set_draw_params(draw_params);

	ego::Program *program = buffer->program();
	program->enable();
	program->uniform("color", color.r, color.g, color.b, color.a);
	program->disable();

	return buffer;
}

Grid::Grid(int x, int y)
    : m_grid_buffer(nullptr)
    , m_xline_buffer(nullptr)
    , m_zline_buffer(nullptr)
{
	/* Set up grid buffer. */

	m_grid_buffer = create_buffer(glm::vec4(1.0f), 1.0f);

	const auto total_vertices = ((x + 1) + (y + 1)) * 2;
	std::vector<glm::vec3> vertices(total_vertices);

	const int half_x = (x / 2), half_z = (y / 2);
	auto count(0);
	for (int i = -half_z; i <= half_z; ++i) {
		vertices[count++] = glm::vec3(i, 0.0f, -half_z);
		vertices[count++] = glm::vec3(i, 0.0f,  half_z);
		vertices[count++] = glm::vec3(-half_x, 0.0f, i);
		vertices[count++] = glm::vec3( half_x, 0.0f, i);
	}

	std::vector<unsigned int> indices(x * y);
	std::iota(indices.begin(), indices.end(), 0);

	m_grid_buffer->set_vertex_buffer("vertex", vertices, indices);

	/* Set line buffers. */

	std::vector<unsigned int> line_indices(2);
	std::iota(line_indices.begin(), line_indices.end(), 0);

	/* Set up X line buffer. */

	m_xline_buffer = create_buffer(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

	std::vector<glm::vec3> xvertices(2);
	xvertices[0] = glm::vec3(-half_x, 0.0f, 0.0f);
	xvertices[1] = glm::vec3( half_x, 0.0f, 0.0f);

	m_xline_buffer->set_vertex_buffer("vertex", xvertices, line_indices);

	/* Set up Z line buffer. */

	m_zline_buffer = create_buffer(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2.0f);

	std::vector<glm::vec3> zvertices(2);
	zvertices[0] = glm::vec3(0.0f, 0.0f, -half_z);
	zvertices[1] = glm::vec3(0.0f, 0.0f,  half_z);

	m_zline_buffer->set_vertex_buffer("vertex", zvertices, line_indices);
}

Grid::~Grid()
{
	delete m_grid_buffer;
	delete m_xline_buffer;
	delete m_zline_buffer;
}

void Grid::render(const ViewerContext &context)
{
	m_grid_buffer->render(context, false);
	m_xline_buffer->render(context, false);
	m_zline_buffer->render(context, false);
}
