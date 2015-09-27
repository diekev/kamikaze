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

#include <fstream>
#include <iostream>
#include <sstream>

#include "GPUShader.h"

GPUShader::GPUShader()
{
	m_total_shaders = 0;
	m_shaders[VERTEX_SHADER] = 0;
	m_shaders[FRAGMENT_SHADER] = 0;
	m_shaders[GEOMETRY_SHADER] = 0;
	m_attrib_list.clear();
	m_uniform_loc_list.clear();
}

GPUShader::~GPUShader()
{
	glDeleteProgram(m_program);

	m_attrib_list.clear();
	m_uniform_loc_list.clear();
}

void GPUShader::loadFromString(GLenum whichShader, const std::string &source)
{
	GLuint shader = glCreateShader(whichShader);
	const char *ptmp = source.c_str();
	glShaderSource(shader, 1, &ptmp, nullptr);

	/* Check whether the shader loads fine or not. */
	GLint status;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog);

		std::cerr << "Compile log: " << infoLog << '\n';
		delete [] infoLog;
	}

	m_shaders[m_total_shaders++] = shader;
}

void GPUShader::loadFromFile(GLenum whichShader, const std::string &source)
{
	std::ifstream fp(source.c_str());

	if (fp) {
		std::stringstream shader_data;
		shader_data << fp.rdbuf();

		fp.close();

		const std::string &shader = shader_data.str();
		loadFromString(whichShader, shader);
	}
	else {
		std::cerr << "Error loading shader: " << source << '\n';
	}
}

void GPUShader::createAndLinkProgram()
{
	m_program = glCreateProgram();

	if (m_shaders[VERTEX_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[VERTEX_SHADER]);
	}
	if (m_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[FRAGMENT_SHADER]);
	}
	if (m_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader(m_program, m_shaders[GEOMETRY_SHADER]);
	}

	/* Link and check whether the program links fine or not. */
	GLint status;
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog = new GLchar[infoLogLength];
		glGetProgramInfoLog(m_program, infoLogLength, nullptr, infoLog);

		std::cerr << "Link log: " << infoLog << '\n';
		delete [] infoLog;
	}

	glDeleteShader(m_shaders[VERTEX_SHADER]);
	glDeleteShader(m_shaders[FRAGMENT_SHADER]);
	glDeleteShader(m_shaders[GEOMETRY_SHADER]);
}

void GPUShader::use()
{
	glUseProgram(m_program);
}

void GPUShader::unUse()
{
	glUseProgram(0);
}

void GPUShader::addAttribute(const std::string &attribute)
{
	m_attrib_list[attribute] = glGetAttribLocation(m_program, attribute.c_str());
}

void GPUShader::addUniform(const std::string &uniform)
{
	m_uniform_loc_list[uniform] = glGetUniformLocation(m_program, uniform.c_str());
}

GLuint GPUShader::operator[](const std::string &attribute)
{
	return m_attrib_list[attribute];
}

GLuint GPUShader::operator()(const std::string &uniform)
{
	return m_uniform_loc_list[uniform];
}
