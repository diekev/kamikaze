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

GPUProgram::GPUProgram()
    : m_total_shaders(0)
    , m_shaders{0, 0, 0}
{}

GPUProgram::~GPUProgram()
{
	if (glIsProgram(m_program)) {
		glDeleteProgram(m_program);
	}

	m_attrib_list.clear();
	m_uniform_loc_list.clear();
}

GPUProgram::UPtr GPUProgram::create()
{
	return UPtr(new GPUProgram());
}

void GPUProgram::loadFromString(GLenum whichShader, const std::string &source)
{
	GLuint shader = glCreateShader(whichShader);
	const char *ptmp = source.c_str();

	glShaderSource(shader, 1, &ptmp, nullptr);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		logError(shader, "Compile log", glGetShaderiv, glGetShaderInfoLog);
	}

	m_shaders[m_total_shaders++] = shader;
}

void GPUProgram::loadFromFile(GLenum whichShader, const std::string &source)
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

void GPUProgram::createAndLinkProgram()
{
	m_program = glCreateProgram();

	for (int i = 0; i < 3; ++i) {
		if (m_shaders[i] != 0) {
			glAttachShader(m_program, m_shaders[i]);
		}
	}

	glLinkProgram(m_program);

	GLint status;
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		logError(m_program, "Linking log", glGetProgramiv, glGetProgramInfoLog);
	}

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

		GLint status;
		glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status);

		if (status == GL_FALSE) {
			logError(m_program, "Validation log", glGetProgramiv, glGetProgramInfoLog);
			return false;
		}

		return true;
	}

	return false;
}

void GPUProgram::addAttribute(const std::string &attribute)
{
	m_attrib_list[attribute] = glGetAttribLocation(m_program, attribute.c_str());
}

void GPUProgram::addUniform(const std::string &uniform)
{
	m_uniform_loc_list[uniform] = glGetUniformLocation(m_program, uniform.c_str());
}

GLuint GPUProgram::operator[](const std::string &attribute)
{
	return m_attrib_list[attribute];
}

GLuint GPUProgram::operator()(const std::string &uniform)
{
	return m_uniform_loc_list[uniform];
}

void GPUProgram::logError(GLuint index, const std::string &prefix,
                          get_ivfunc ivfunc, get_logfunc log_func) const
{
	GLint log_length;
	ivfunc(index, GL_INFO_LOG_LENGTH, &log_length);

	auto log = std::unique_ptr<char[]>(new char[log_length]);
	log_func(index, log_length, &log_length, log.get());

	std::cerr << prefix << ": " << log.get() << '\n';
}
