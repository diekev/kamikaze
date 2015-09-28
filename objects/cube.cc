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

#include "render/GPUShader.h"
#include "util/util_opengl.h"

#include "cube.h"
#include "render/GPUBuffer.h"

Cube::Cube(const glm::vec3 &min, const glm::vec3 &max)
    : m_buffer_data(new GPUBuffer)
{
	m_shader.loadFromFile(GL_VERTEX_SHADER, "shader/flat_shader.vert");
	m_shader.loadFromFile(GL_FRAGMENT_SHADER, "shader/flat_shader.frag");

	m_shader.createAndLinkProgram();

	m_shader.use();
	{
		m_shader.addAttribute("vertex");
		m_shader.addUniform("MVP");
	}
	m_shader.unUse();

	const glm::vec3 vertices[8] = {
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

	m_buffer_data->bind();
	m_buffer_data->create_vertex_buffer(&vertices[0][0], sizeof(vertices));
	m_buffer_data->create_index_buffer(&indices[0], sizeof(indices));
	m_buffer_data->attrib_pointer(m_shader["vertex"], 3);
	m_buffer_data->unbind();
}

Cube::~Cube()
{
	delete m_buffer_data;
}

void Cube::render(const glm::mat4 &MVP)
{
	glEnable(GL_DEPTH_TEST);

	m_shader.use();
	{
		m_buffer_data->bind();

		glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);

		m_buffer_data->unbind();
	}
	m_shader.unUse();

	glDisable(GL_DEPTH_TEST);
}
