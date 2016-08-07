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

#pragma once

#include <string>

template <typename OpType>
bool ensure_unique_name(std::string &name, const OpType &op)
{
	if (op(name)) {
		return false;
	}

	std::string temp = name + ".0000";
	const auto temp_size = temp.size();
	int number = 0;

	do {
		++number;

		if (number < 10) {
			temp[temp_size - 1] = '0' + number;
		}
		else if (number < 100) {
			temp[temp_size - 1] = '0' + (number % 10);
			temp[temp_size - 2] = '0' + (number / 10);
		}
		else if (number < 1000) {
			temp[temp_size - 1] = '0' + (number % 10);
			temp[temp_size - 2] = '0' + ((number % 100) / 10);
			temp[temp_size - 3] = '0' + (number / 100);
		}
		else {
			temp[temp_size - 1] = '0' + (number % 10);
			temp[temp_size - 2] = '0' + ((number % 100) / 10);
			temp[temp_size - 3] = '0' + ((number % 1000) / 100);
			temp[temp_size - 4] = '0' + (number / 1000);
		}
	} while (!op(temp));

	name = temp;
	return true;
}
