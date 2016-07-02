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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "any.h"

any::any(const any &other)
    : any()
{
	if (other.dt) {
		dt = other.dt->copy();
	}
}

any::any(any &&other) noexcept
    : any()
{
	std::swap(this->dt, other.dt);
}

any::~any()
{
	clear();
}

any &any::operator=(any &&other) noexcept
{
	this->swap(other);
	return *this;
}

void any::clear()
{
	delete dt;
	dt = nullptr;
}

void any::swap(any &other) noexcept
{
	std::swap(this->dt, other.dt);
}

bool any::empty() const noexcept
{
	return dt == nullptr;
}

const std::type_info &any::type() const noexcept
{
	if (this->empty()) {
		return typeid(void);
	}

	return dt->type();
}

any &any::operator=(const any &other)
{
	if (&other == this) {
		return *this;
	}

	if (!this->empty()) {
		this->clear();
	}

	if (other.dt) {
		this->dt = other.dt->copy();
	}

	return *this;
}

const char *bad_any_cast::what() const noexcept
{
	return "Bad any_cast\n";
}
