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

#include <GL/glew.h>
#include <map>
#include <string>

class GPUShader {
	enum ShadeType {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER
	};

	GLuint m_program;
	int m_total_shaders;
	GLuint m_shaders[3];
	std::map<std::string, GLuint> m_attrib_list;
	std::map<std::string, GLuint> m_uniform_loc_list;

public:
	GPUShader();
	~GPUShader();

	void loadFromString(GLenum whichShader, const std::string &source);
	void loadFromFile(GLenum whichShader, const std::string &source);
	void createAndLinkProgram();
	void use();
	void unUse();
	void addAttribute(const std::string &attribute);
	void addUniform(const std::string &uniform);

	GLuint operator[](const std::string &attribute);
	GLuint operator()(const std::string &uniform);
};
