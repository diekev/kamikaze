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

#include <cassert>
#include <GL/glew.h>

#include "GPUTexture.h"

GPUTexture::GPUTexture(GLenum target, GLint texture)
    : m_index(0)
    , m_internal_format(0)
    , m_border(0)
    , m_texture(texture)
    , m_format(0)
    , m_target(target)
    , m_type(0)
{
	glGenTextures(1, &m_index);
}

GPUTexture::~GPUTexture()
{
	free(false);
}

void GPUTexture::free(bool renew)
{
	if (glIsTexture(m_index)) {
		glDeleteTextures(1, &m_index);
	}

	if (renew) {
		glGenTextures(1, &m_index);
	}
}

void GPUTexture::bind()
{
	glActiveTexture(GL_TEXTURE0 + m_texture);
	glBindTexture(m_target, m_index);
}

void GPUTexture::unbind()
{

	glActiveTexture(GL_TEXTURE0 + m_texture);
	glBindTexture(m_target, 0);
}

void GPUTexture::setType(GLenum type, GLenum format, GLint internal_format)
{
	m_type = type;
	m_format = format;
	m_internal_format = internal_format;
}

void GPUTexture::setMinMagFilter(GLint min, GLint mag)
{
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, mag);
}

void GPUTexture::setWrapping(GLint wrap)
{
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrap);

	if (m_target == GL_TEXTURE_2D) {
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrap);
	}

	if (m_target == GL_TEXTURE_3D) {
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_R, wrap);
	}
}

void GPUTexture::create(const GLvoid *data, const int size)
{
	glTexImage1D(m_target, 0, m_internal_format, size, 0, m_format, m_type, data);
}

void GPUTexture::create2D(const GLvoid *data, const int size[2])
{
	glTexImage2D(m_target, 0, m_internal_format,
	             size[0], size[1], 0, m_format, m_type, data);
}

void GPUTexture::create3D(const GLvoid *data, const int size[3])
{
	glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, 4);

	glTexImage3D(m_target, 0, m_internal_format,
	             size[0], size[1], size[2], 0, m_format, m_type, data);

	glGenerateMipmap(GL_TEXTURE_3D);
}

GLint GPUTexture::unit() const
{
	return m_texture;
}
