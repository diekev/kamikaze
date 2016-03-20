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

#include <string>

namespace filesystem {

/* same values as in dlfcn.h, avoids including the header here, type safety... */
enum class dso_loading {
	local     = 0,       /* RTLD_LOCAL */
	lazy      = 0x00001, /* RTLD_LAZY */
	now       = 0x00002, /* RTLD_NOW */
	no_load   = 0x00004, /* RTLD_NOLOAD */
	deep_bind = 0x00008, /* RTLD_DEEPBIND */
	global    = 0x00100, /* RTLD_GLOBAL */
	no_delete = 0x01000, /* RTLD_NODELETE */
};

class shared_library {
	void *m_handle = nullptr;
	std::string m_path{};

public:
	shared_library() = default;

	shared_library(const std::string &filename,
	               dso_loading flag = dso_loading::lazy) noexcept;

	~shared_library() noexcept;

	shared_library(const shared_library &) = delete;
	shared_library &operator=(const shared_library &) = delete;

	shared_library(shared_library &&other);
	shared_library &operator=(shared_library &&other);

	void swap(shared_library &other);

	void open(const std::string &filename,
	          dso_loading flag = dso_loading::lazy) noexcept;

	/**
	 * Return the address, cast to typename T, of where the symbol referenced by
	 * name is loaded into memory.
	 */
	template <typename T>
	auto symbol(const std::string &name) const noexcept -> T;

	/**
	 * Find the first occurrence of the desired symbol using the default library
	 * search order.
	 */
	template <typename T>
	static auto symbol_default(const std::string &name) noexcept -> T;

	/**
	 * Find the next occurrence of a function in the search order after the
	 * current library.
	 */
	template <typename T>
	static auto symbol_next(const std::string &name) noexcept -> T;

	char *error() const noexcept;

	explicit operator bool() const noexcept;
};

namespace __detail {

void *get_symbol(void *handle, const std::string &name) noexcept;

}  /* namespace __detail */

template <typename T>
auto shared_library::symbol(const std::string &name) const noexcept -> T
{
	auto ptr = __detail::get_symbol(m_handle, name);

	if (ptr == nullptr) {
		return nullptr;
	}

	return reinterpret_cast<T>(ptr);
}

template <typename T>
auto shared_library::symbol_default(const std::string &name) noexcept -> T
{
	/* RTLD_DEFAULT = (void *)0 */
	auto ptr = __detail::get_symbol(static_cast<void *>(0), name);

	if (ptr == nullptr) {
		return nullptr;
	}

	return reinterpret_cast<T>(ptr);
}

template <typename T>
auto shared_library::symbol_next(const std::string &name) noexcept -> T
{
	/* RTLD_NEXT = (void *)-1 */
	auto ptr = __detail::get_symbol(reinterpret_cast<void *>(-1), name);

	if (ptr == nullptr) {
		return nullptr;
	}

	return reinterpret_cast<T>(ptr);
}

}  /* namespace filesystem */
