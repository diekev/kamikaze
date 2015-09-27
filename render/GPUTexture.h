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

class GPUTexture {
	GLuint m_index;
	GLint m_internal_format;
	GLint m_border;
	GLint m_texture;
	GLenum m_format;
	GLenum m_target;
	GLenum m_type;

public:
	GPUTexture(GLenum target, GLint texture);
	~GPUTexture();

	void free(bool renew);
	void bind();
	void unbind();

	void setType(GLenum type, GLenum format, GLint internal_format);
	void setMinMagFilter(GLint min, GLint mag);
	void setWrapping(GLint wrap);

	void create(const GLvoid *data, const int size);
	void create2D(const GLvoid *data, const int size[2]);
	void create3D(const GLvoid *data, const int size[3]);

	GLint unit() const;
};
