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

GPUTexture::UPtr GPUTexture::create(GLenum target, GLint texture)
{
	return UPtr(new GPUTexture(target, texture));
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

void GPUTexture::bind() const
{
	glActiveTexture(GL_TEXTURE0 + m_texture);
	glBindTexture(m_target, m_index);
}

void GPUTexture::unbind() const
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

void GPUTexture::setMinMagFilter(GLint min, GLint mag) const
{
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, mag);
}

void GPUTexture::setWrapping(GLint wrap) const
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

void GPUTexture::generateMipMap(GLint base, GLint max) const
{
	glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, base);
	glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, max);
	glGenerateMipmap(m_target);
}

void GPUTexture::createTexture(const GLvoid *data, GLint *size) const
{
	if (m_target == GL_TEXTURE_1D) {
		glTexImage1D(m_target, 0, m_internal_format, size[0], m_border, m_format,
		             m_type, data);
	}
	else if (m_target == GL_TEXTURE_2D) {
		glTexImage2D(m_target, 0, m_internal_format, size[0], size[1], m_border,
		             m_format, m_type, data);
	}
	else {
		glTexImage3D(m_target, 0, m_internal_format, size[0], size[1], size[2],
		             m_border, m_format, m_type, data);
	}
}

void GPUTexture::createSubImage(const GLvoid *data, GLint *size, GLint *offset) const
{
	if (m_target == GL_TEXTURE_1D) {
		glTexSubImage1D(m_target, 0, offset[0], size[0], m_format, m_type, data);
	}
	else if (m_target == GL_TEXTURE_2D) {
		glTexSubImage2D(m_target, 0, offset[0], offset[1], size[0], size[1],
		                m_format, m_type, data);
	}
	else {
		glTexSubImage3D(m_target, 0, offset[0], offset[1], offset[2],
		                size[0], size[1], size[2], m_format, m_type, data);
	}
}

GLint GPUTexture::unit() const
{
	return m_texture;
}
