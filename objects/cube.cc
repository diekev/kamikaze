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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/GLSLShader.h"

#include "cube.h"

Cube::Cube(const glm::vec3 &min, const glm::vec3 &max)
{
	m_shader.loadFromFile(GL_VERTEX_SHADER, "shader/flat_shader.vert");
	m_shader.loadFromFile(GL_FRAGMENT_SHADER, "shader/flat_shader.frag");

	m_shader.createAndLinkProgram();

	m_shader.use();
	{
		m_shader.addAttribute("vVertex");
		m_shader.addUniform("MVP");
	}
	m_shader.unUse();

	glm::vec3 vertices[8] = {
	    glm::vec3(min[0], min[1], min[2]),
	    glm::vec3(max[0], min[1], min[2]),
	    glm::vec3(max[0], max[1], min[2]),
	    glm::vec3(min[0], max[1], min[2]),
	    glm::vec3(min[0], min[1], max[2]),
	    glm::vec3(max[0], min[1], max[2]),
	    glm::vec3(max[0], max[1], max[2]),
	    glm::vec3(min[0], max[1], max[2])
	};

	const GLushort indices[24] = {
	    0, 1, 1, 2,
	    2, 3, 3, 0,
	    4, 5, 5, 6,
	    6, 7, 7, 4,
	    0, 4, 1, 5,
	    2, 6, 3, 7
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_verts_vbo);
	glGenBuffers(1, &m_index_vbo);

	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_verts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);
	glEnableVertexAttribArray(m_shader["vVertex"]);
	glVertexAttribPointer(m_shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Cube::~Cube()
{
	m_shader.deleteShaderProgram();

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_index_vbo);
	glDeleteBuffers(1, &m_verts_vbo);
}

void Cube::render(const glm::mat4 &MVP)
{
	m_shader.use();

	glBindVertexArray(m_vao);
	glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);

	m_shader.unUse();
}
