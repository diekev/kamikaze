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

#include <algorithm>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/GLSLShader.h"
#include "util/util_opengl.h"
#include "grid.h"

Grid::Grid(int x, int y)
    : m_total_indices(x * y)
    , m_shader(new GLSLShader)
{
	m_shader->loadFromFile(GL_VERTEX_SHADER, "shader/flat_shader.vert");
	m_shader->loadFromFile(GL_FRAGMENT_SHADER, "shader/flat_shader.frag");

	m_shader->createAndLinkProgram();

	m_shader->use();
	{
		m_shader->addAttribute("vertex");
		m_shader->addUniform("MVP");
	}
	m_shader->unUse();

	/* setup vertex buffer */

	auto total_vertices = ((x + 1) + (y + 1)) * 2;
	std::vector<glm::vec3> vertices;
	vertices.resize(total_vertices);

	int half_x = x >> 1, half_y = y >> 1;
	auto count(0);
	for (int i = -half_y; i <= half_y; ++i) {
		vertices[count++] = glm::vec3(i, 0.0f, -half_y);
		vertices[count++] = glm::vec3(i, 0.0f,  half_y);
		vertices[count++] = glm::vec3(-half_x, 0.0f, i);
		vertices[count++] = glm::vec3( half_x, 0.0f, i);
	}

	std::vector<GLuint> indices;
	indices.resize(m_total_indices);
	std::iota(indices.begin(), indices.end(), 0);

	const auto &vsize = total_vertices * sizeof(glm::vec3);
	const auto &isize = m_total_indices * sizeof(GLuint);

	m_buffer_data = create_vertex_buffers((*m_shader)["vertex"], &(vertices[0].x), vsize, &indices[0], isize);
}

Grid::~Grid()
{
	delete m_shader;
	delete_vertex_buffers(m_buffer_data);
}

void Grid::render(const glm::mat4 &MVP)
{
	glEnable(GL_DEPTH_TEST);

	m_shader->use();
	{
		glBindVertexArray(m_buffer_data->vao);

		glUniformMatrix4fv((*m_shader)("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glDrawElements(GL_LINES, m_total_indices, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);
	}
	m_shader->unUse();

	glDisable(GL_DEPTH_TEST);
}
