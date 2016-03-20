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

#include <dirent.h>
#include <string>
#include <vector>

namespace filesystem {

class directory_iterator {
	std::string m_path{};
	std::string m_current{};
	DIR *m_dir = nullptr;

public:
	directory_iterator() = default;
	directory_iterator(const std::string &filename);

	directory_iterator(const directory_iterator &other);
	~directory_iterator() noexcept;

	directory_iterator &operator=(const directory_iterator &other) = default;

	directory_iterator &operator++();

	const std::string &operator*() const noexcept;
	const std::string *operator->() const noexcept;
};

bool operator==(const directory_iterator &ita, const directory_iterator &itb) noexcept;
bool operator!=(const directory_iterator &ita, const directory_iterator &itb) noexcept;

class dir {
	DIR *m_dir = nullptr;
	std::string m_path{};

public:
	dir() = default;

	dir(const std::string &filename);

	dir(const dir &) = delete;
	dir &operator=(const dir &) = delete;

	~dir() noexcept;

	using iterator = directory_iterator;

	iterator begin();
	iterator end();

	bool read_next(std::string &filename) const noexcept;

	long tell() const noexcept;

	void rewind() const noexcept;

	void seek(long location) const noexcept;

	std::vector<std::string> scan() const noexcept;

	explicit operator bool() const noexcept;
};

bool is_library(const std::string &path);

bool is_relative(const std::string &path);

std::string filename(const std::string &path);

std::string directory(const std::string &path);

}  /* namespace filesystem */
