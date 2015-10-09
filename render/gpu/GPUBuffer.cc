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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include <GL/glew.h>

#include "GPUBuffer.h"

GPUBuffer::GPUBuffer()
{
	glGenVertexArrays(1, &m_vao);
}

GPUBuffer::~GPUBuffer()
{
	glDeleteVertexArrays(1, &m_vao);

	if (glIsBuffer(m_vertex_buffer)) {
		glDeleteBuffers(1, &m_vertex_buffer);
	}

	if (glIsBuffer(m_index_buffer)) {
		glDeleteBuffers(1, &m_index_buffer);
	}

	if (glIsBuffer(m_normal_buffer)) {
		glDeleteBuffers(1, &m_normal_buffer);
	}
}

GPUBuffer::UPtr GPUBuffer::create()
{
	return UPtr(new GPUBuffer());
}

void GPUBuffer::bind() const
{
	glBindVertexArray(m_vao);
}

void GPUBuffer::unbind() const
{
	glBindVertexArray(0);
}

void GPUBuffer::attribPointer(GLuint index, GLint size) const
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void GPUBuffer::generateVertexBuffer(const GLvoid *vertices, const size_t size)
{
	generateBuffer(m_vertex_buffer, vertices, size, GL_ARRAY_BUFFER);
}

void GPUBuffer::generateIndexBuffer(const GLvoid *indices, const size_t size)
{
	generateBuffer(m_index_buffer, indices, size, GL_ELEMENT_ARRAY_BUFFER);
}

void GPUBuffer::generateNormalBuffer(const GLvoid *colors, const size_t size)
{
	generateBuffer(m_normal_buffer, colors, size, GL_ARRAY_BUFFER);
}

void GPUBuffer::generateBuffer(GLuint &id, const GLvoid *data, const size_t size, GLenum target) const
{
	if (glIsBuffer(id)) {
		glDeleteBuffers(1, &id);
	}

	GLenum draw_type = GL_STATIC_DRAW;

	// TODO: feels a bit hackish
	if (data == nullptr) {
		draw_type = GL_DYNAMIC_DRAW;
	}

	glGenBuffers(1, &id);

	glBindBuffer(target, id);
	glBufferData(target, size, data, draw_type);
}

void GPUBuffer::updateVertexBuffer(const GLvoid *vertices, const size_t size) const
{
	if (glIsBuffer(m_vertex_buffer)) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);
	}
}

void GPUBuffer::updateIndexBuffer(const GLvoid *indices, const size_t size) const
{
	if (glIsBuffer(m_index_buffer)) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, indices);
	}
}
