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

#include "dynamiclibrary.h"

#include <dlfcn.h>
#include <iostream>

namespace filesystem {

namespace __detail {

void *get_symbol(void *handle, const std::string &name) noexcept
{
	dlerror();  /* clear any existing error */

	const auto sym = dlsym(handle, name.c_str());

	const auto err = dlerror();
	if (err != nullptr) {
		std::cerr << err << '\n';
		return nullptr;
	}

	return sym;
}

}  /* namespace __detail */

shared_library::shared_library(const std::string &filename, dso_loading flag) noexcept
    : shared_library()
{
	open(filename, flag);
}

shared_library::shared_library(shared_library &&other)
    : shared_library()
{
	swap(other);
}

shared_library &shared_library::operator=(shared_library &&other)
{
	swap(other);
	return *this;
}

shared_library::~shared_library() noexcept
{
	if (m_handle == nullptr) {
		return;
	}

	if (dlclose(m_handle) != 0) {
		std::cerr << error() << '\n';
	}
}

void shared_library::swap(shared_library &other)
{
	using std::swap;
	swap(m_handle, other.m_handle);
	swap(m_path, other.m_path);
}

void shared_library::open(const std::string &filename, dso_loading flag) noexcept
{
	if (m_path == filename) {
		return;
	}

	if (m_handle != nullptr) {
		dlclose(m_handle);
	}

	m_handle = dlopen(filename.c_str(), static_cast<int>(flag));
	m_path = filename;
}

char *shared_library::error() const noexcept
{
	return dlerror();
}

shared_library::operator bool() const noexcept
{
	return (m_handle != nullptr);
}

}  /* namespace filesystem */
