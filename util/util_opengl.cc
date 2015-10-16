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

#include <cassert>
#include <GL/glew.h>
#include <iostream>
#include <memory>

#include "util_opengl.h"

void gl_check_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR) {
		return;
	}

	std::cerr << "GL Error: " << int(error) << " - ";

	switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << "invalid enum.\n";
			break;
		case GL_INVALID_VALUE:
			std::cerr << "invalid value.\n";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << "invalid operation.\n";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << "out of memory.\n";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << "invalid framebuffer operation\n";
			break;
	}

	/* Cause a crash if an error was caught. */
	assert(error == GL_NO_ERROR);
}

/* Utility function to check whether a program or shader was linked properly or
 * not. Prints an error message if linking failed. */
bool check_status(GLuint index, GLenum pname, const std::string &prefix,
                  get_ivfunc ivfunc, get_logfunc log_func)
{
	GLint status;
	ivfunc(index, pname, &status);

	if (status == GL_TRUE) {
		return true;
	}

	GLint log_length;
	ivfunc(index, GL_INFO_LOG_LENGTH, &log_length);

	auto log = std::unique_ptr<char[]>(new char[log_length]);
	log_func(index, log_length, &log_length, log.get());

	std::cerr << prefix << ": " << log.get() << '\n';

	return false;
}
