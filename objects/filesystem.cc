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

#include "filesystem.h"

#include <cassert>

namespace filesystem {

/* *************************** directory iterator *************************** */

directory_iterator::directory_iterator(const std::string &filename)
    : m_path(filename)
    , m_current(filename)
{
	if (!m_path.empty()) {
		m_dir = opendir(m_path.c_str());

		if (m_dir == nullptr) {
			std::perror("");
		}
	}
}

directory_iterator::directory_iterator(const directory_iterator &other)
    : directory_iterator(other.m_path)
{}

directory_iterator::~directory_iterator() noexcept
{
	if (m_dir && closedir(m_dir) != 0) {
		std::perror("");
	}
}

directory_iterator &directory_iterator::operator++()
{
	struct dirent *ent = readdir(m_dir);

	if (ent != nullptr) {
		m_current = m_path + ent->d_name;
	}
	else {
		m_current = "";
	}

	return (*this);
}

const std::string &directory_iterator::operator*() const noexcept
{
	return m_current;
}

const std::string *directory_iterator::operator->() const noexcept
{
	return &m_current;
}

bool operator==(const directory_iterator &ita, const directory_iterator &itb) noexcept
{
	return (*ita == *itb);
}

bool operator!=(const directory_iterator &ita, const directory_iterator &itb) noexcept
{
	return !(ita == itb);
}

/* ******************************** directory ******************************* */

dir::operator bool() const noexcept
{
	return (m_dir != nullptr);
}

dir::dir(const std::string &filename)
    : dir()
{
	m_dir = opendir(filename.c_str());
	m_path = filename;

	if (m_dir == nullptr) {
		std::perror("");
	}
}

dir::~dir() noexcept
{
	if (closedir(m_dir) != 0) {
		std::perror("");
	}
}

dir::iterator dir::begin()
{
	assert(!m_path.empty());
	return directory_iterator(m_path);
}

dir::iterator dir::end()
{
	return directory_iterator("");
}

bool dir::read_next(std::string &filename) const noexcept
{
	struct dirent *ent = readdir(m_dir);

	if (ent != nullptr) {
		filename = m_path + ent->d_name;
		return true;
	}

	return false;
}

long dir::tell() const noexcept
{
	return telldir(m_dir);
}

void dir::rewind() const noexcept
{
	rewinddir(m_dir);
}

void dir::seek(long location) const noexcept
{
	seekdir(m_dir, location);
}

std::vector<std::string> dir::scan() const noexcept
{
	struct dirent **namelist;
	int n = scandir(m_path.c_str(), &namelist, nullptr, alphasort);

	if (n < 0) {
		std::perror("scandir");
		return {};
	}

	std::vector<std::string> result;
	result.reserve(n);

	while (n--) {
		result.emplace_back(namelist[n]->d_name);
		free(namelist[n]);
	}

	free(namelist);

	return result;
}

std::string filename(const std::string &path)
{
	const auto idx = path.find_last_of('/');
	return path.substr(idx + 1);
}

std::string directory(const std::string &path)
{
	const auto idx = path.find_last_of('/');
	return path.substr(0, idx);
}

bool is_relative(const std::string &path)
{
	return path[0] == '.' && (path[1] == '.' || path[1] == '/');
}

bool is_library(const std::string &filename)
{
	const auto index = filename.find(".so");
	return (index != std::string::npos) && (index == (filename.size() - 3));
}

}  /* namespace filesystem */

#if 0
void file_types(const std::string &path)
{
	struct dirent **namelist;
	int n = scandir(path.c_str(), &namelist, nullptr, alphasort);

	while (n--) {
		std::string sFileType;
		struct dirent *currentfile = namelist[n];

		auto fileType = currentfile->d_type;

		if (fileType == DT_REG) {
			sFileType = "File";
		}
		else if (fileType == DT_DIR) {
			sFileType = "Directory";
		//isDirectory = true;
		}
		else if (fileType == DT_FIFO) {
			sFileType = "FIFO";
		}
		else if (fileType == DT_SOCK) {
			sFileType = "Local socket";
		}
		else if (fileType == DT_CHR) {
			sFileType = "Character device";
		}
		else if (fileType == DT_BLK) {
			sFileType = "Block device";
		}
		else if( fileType == DT_UNKNOWN ) {
			sFileType = "Unknown file type";
		}
		else {
			sFileType = "Unknown";
		}

		std::cout << currentfile->d_name << ": " << sFileType << '\n';
		free(currentfile);
	}

	free(namelist);
}
#endif
