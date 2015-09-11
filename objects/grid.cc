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
#include "grid.h"

Grid::Grid(int x, int y)
    : m_total_indices(x * y)
    , m_shader(new GLSLShader)
{
	m_shader->loadFromFile(GL_VERTEX_SHADER, "shader/flat_shader.vert");
	m_shader->loadFromFile(GL_FRAGMENT_SHADER, "shader/flat_shader.frag");

	m_shader->createAndLinkProgram();

	m_shader->use();

	m_shader->addAttribute("vertex");
	m_shader->addUniform("MVP");

	m_shader->unUse();

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo_id);
	glGenBuffers(1, &m_index_vbo_id);

	glBindVertexArray(m_vao);

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

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, total_vertices * sizeof(glm::vec3), &(vertices[0].x), GL_STATIC_DRAW);
	glEnableVertexAttribArray((*m_shader)["vertex"]);
	glVertexAttribPointer((*m_shader)["vertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* setup index buffer */

	std::vector<GLuint> indices;
	indices.resize(m_total_indices);
	std::iota(indices.begin(), indices.end(), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_vbo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_total_indices * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Grid::~Grid()
{
	m_shader->deleteShaderProgram();
	delete m_shader;

	glDeleteBuffers(1, &m_vbo_id);
	glDeleteBuffers(1, &m_index_vbo_id);
	glDeleteVertexArrays(1, &m_vao);
}

void Grid::render(const glm::mat4 &MVP)
{
	m_shader->use();

	glBindVertexArray(m_vao);
	glUniformMatrix4fv((*m_shader)("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glDrawElements(GL_LINES, m_total_indices, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	m_shader->unUse();
}
