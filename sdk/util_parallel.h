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

#include <tbb/parallel_for.h>

/**
 * Wrappers around Intel's TBB utilities.
 * Inspired by "Multithreading for Visual Effects", chapter 2.
 */

template <typename RangeType>
inline int range_size(const RangeType &range)
{
	return range.end() - range.begin();
}

template <typename RangeType, typename OpType>
inline void serial_for(RangeType &&range, OpType &&op)
{
	op(range);
}

template <typename RangeType, typename OpType>
void parallel_for(RangeType &&range, OpType &&op, int grain_size = 1)
{
	const auto size = range_size(range);

	if (size == 0) {
		return;
	}

	/* Check if a single task can be created. */
	if (size <= grain_size) {
		serial_for(range, op);
		return;
	}

	tbb::parallel_for(RangeType(range.begin(), range.end(), grain_size), op);
}

template <typename RangeType, typename OpType>
inline void parallel_for_light_items(RangeType &&range, OpType &&op)
{
	parallel_for(range, op, 1024);
}

template <typename RangeType, typename OpType>
inline void parallel_for_heavy_items(RangeType &&range, OpType &&op)
{
	parallel_for(range, op, 1);
}
