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

#pragma once

class GPUBuffer {
	GLuint m_vao;
	GLuint m_vertex_buffer;
	GLuint m_index_buffer;
	GLuint m_color_buffer;

	void create_buffer(GLuint &id, const GLvoid *data, const size_t size, GLenum target);

public:
	GPUBuffer();
	~GPUBuffer();

	void bind();
	void unbind();

	void attrib_pointer(GLuint index, GLint size);
	void create_vertex_buffer(const GLvoid *vertices, const size_t size);
	void update_vertex_buffer(const GLvoid *vertices, const size_t size);
	void create_index_buffer(const GLvoid *indices, const size_t size);
	void update_index_buffer(const GLvoid *indices, const size_t size);
	void create_color_buffer(const GLvoid *colors, const size_t size);
};
