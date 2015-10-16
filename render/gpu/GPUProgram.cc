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
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "GPUProgram.h"

#include "util/util_opengl.h"

GPUProgram::GPUProgram()
    : m_shaders{0, 0, 0}
{}

GPUProgram::~GPUProgram()
{
	if (glIsProgram(m_program)) {
		glDeleteProgram(m_program);
	}

	m_attributes.clear();
	m_uniforms.clear();
}

GPUProgram::UPtr GPUProgram::create()
{
	return UPtr(new GPUProgram());
}

void GPUProgram::loadFromString(int shader_type, const std::string &source)
{
	GLenum type;

	switch (shader_type) {
		case VERTEX_SHADER:
			type = GL_VERTEX_SHADER;
			break;
		case FRAGMENT_SHADER:
			type = GL_FRAGMENT_SHADER;
			break;
		case GEOMETRY_SHADER:
			type = GL_GEOMETRY_SHADER;
			break;
		default:
			assert(0);
			break;
	}

	GLuint shader = glCreateShader(type);
	const char *ptmp = source.c_str();

	glShaderSource(shader, 1, &ptmp, nullptr);
	glCompileShader(shader);

	const bool ok = check_status(shader, GL_COMPILE_STATUS, "Compile log",
	                             glGetShaderiv, glGetShaderInfoLog);

	if (ok) {
		m_shaders[shader_type] = shader;
	}
}

void GPUProgram::loadFromFile(int shader_type, const std::string &source)
{
	std::ifstream fp(source.c_str());

	if (!fp) {
		std::cerr << "Error loading shader: " << source << '\n';
		return;
	}

	std::stringstream shader_data;
	shader_data << fp.rdbuf();
	fp.close();

	const std::string &shader = shader_data.str();
	loadFromString(shader_type, shader);
}

void GPUProgram::createAndLinkProgram()
{
	m_program = glCreateProgram();

	for (int i = 0; i < 3; ++i) {
		if (glIsShader(m_shaders[i])) {
			glAttachShader(m_program, m_shaders[i]);
		}
	}

	glLinkProgram(m_program);

	check_status(m_program, GL_LINK_STATUS, "Linking log",
	             glGetProgramiv, glGetProgramInfoLog);

	for (int i = 0; i < 3; ++i) {
		if (glIsShader(m_shaders[i])) {
			glDetachShader(m_program, m_shaders[i]);
			glDeleteShader(m_shaders[i]);
		}
	}
}

void GPUProgram::enable() const
{
	glUseProgram(m_program);
}

void GPUProgram::disable() const
{
	glUseProgram(0);
}

bool GPUProgram::isValid() const
{
	if (glIsProgram(m_program)) {
		glValidateProgram(m_program);

		return check_status(m_program, GL_VALIDATE_STATUS, "Validation log",
		                    glGetProgramiv, glGetProgramInfoLog);
	}

	return false;
}

void GPUProgram::addAttribute(const std::string &attribute)
{
	m_attributes[attribute] = glGetAttribLocation(m_program, attribute.c_str());
}

void GPUProgram::addUniform(const std::string &uniform)
{
	m_uniforms[uniform] = glGetUniformLocation(m_program, uniform.c_str());
}

GLuint GPUProgram::operator[](const std::string &attribute)
{
	return m_attributes[attribute];
}

GLuint GPUProgram::operator()(const std::string &uniform)
{
	return m_uniforms[uniform];
}
