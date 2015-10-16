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

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "grid.h"

#include "util/util_opengl.h"

Grid::Grid(int x, int y)
    : m_buffer_data(std::unique_ptr<GPUBuffer>(new GPUBuffer()))
    , m_elements(x * y)
{
	m_program.loadFromFile(VERTEX_SHADER, "shaders/flat_shader.vert");
	m_program.loadFromFile(FRAGMENT_SHADER, "shaders/flat_shader.frag");

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addUniform("MVP");
		m_program.addUniform("matrix");
	}
	m_program.disable();

	/* setup vertex buffer */

	const auto total_vertices = ((x + 1) + (y + 1)) * 2;
	std::vector<glm::vec3> vertices(total_vertices);

	const int half_x = (x / 2), half_y = (y / 2);
	auto count(0);
	for (int i = -half_y; i <= half_y; ++i) {
		vertices[count++] = glm::vec3(i, 0.0f, -half_y);
		vertices[count++] = glm::vec3(i, 0.0f,  half_y);
		vertices[count++] = glm::vec3(-half_x, 0.0f, i);
		vertices[count++] = glm::vec3( half_x, 0.0f, i);
	}

	std::vector<GLushort> indices(m_elements);
	std::iota(indices.begin(), indices.end(), 0);

	const auto &vsize = total_vertices * sizeof(glm::vec3);
	const auto &isize = m_elements * sizeof(GLushort);

	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&(vertices[0].x), vsize);
	m_buffer_data->generateIndexBuffer(&indices[0], isize);
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->unbind();
}

void Grid::render(const glm::mat4 &MVP)
{
	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glDrawElements(GL_LINES, m_elements, GL_UNSIGNED_SHORT, nullptr);

		m_buffer_data->unbind();
		m_program.disable();
	}
}
