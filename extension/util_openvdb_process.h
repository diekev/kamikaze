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

#pragma once

#include <openvdb/openvdb.h>

/* Utility functions to process grids whose types are unkown when manipulated */

enum {
	GRID_STORAGE_FLOAT  = 0,
	GRID_STORAGE_DOUBLE,
	GRID_STORAGE_BOOL,
	GRID_STORAGE_INT32,
	GRID_STORAGE_INT64,
	GRID_STORAGE_VEC3I,
	GRID_STORAGE_VEC3S,
	GRID_STORAGE_VEC3D,
};

inline int get_grid_storage(const openvdb::GridBase &grid)
{
	if (grid.isType<openvdb::FloatGrid>()) {
		return GRID_STORAGE_FLOAT;
	}
	else if (grid.isType<openvdb::DoubleGrid>()) {
		return GRID_STORAGE_DOUBLE;
	}
	else if (grid.isType<openvdb::BoolGrid>()) {
		return GRID_STORAGE_BOOL;
	}
	else if (grid.isType<openvdb::Int32Grid>()) {
		return GRID_STORAGE_INT32;
	}
	else if (grid.isType<openvdb::Int64Grid>()) {
		return GRID_STORAGE_INT64;
	}
	else if (grid.isType<openvdb::Vec3IGrid>()) {
		return GRID_STORAGE_VEC3I;
	}
	else if (grid.isType<openvdb::Vec3SGrid>()) {
		return GRID_STORAGE_VEC3S;
	}
	else if (grid.isType<openvdb::Vec3DGrid>()) {
		return GRID_STORAGE_VEC3D;
	}

	return -1;
}

/* Functions to help casting an openvdb::Gridbase to the right type */

template <typename GridType>
inline const GridType *VDB_grid_cast(const openvdb::GridBase *grid)
{
	return static_cast<const GridType *>(grid);
}

template <typename GridType>
inline GridType *VDB_grid_cast(openvdb::GridBase *grid)
{
	return static_cast<GridType *>(grid);
}

template <typename GridType>
inline const GridType &VDB_grid_cast(const openvdb::GridBase &grid)
{
	return *static_cast<const GridType *>(&grid);
}

template <typename GridType>
inline GridType &VDB_grid_cast(openvdb::GridBase &grid)
{
	return *static_cast<GridType *>(&grid);
}

template <typename GridType>
inline typename GridType::ConstPtr VDB_grid_cast(openvdb::GridBase::ConstPtr grid)
{
	return openvdb::gridConstPtrCast<GridType>(grid);
}

template <typename GridType>
inline typename GridType::Ptr VDB_grid_cast(openvdb::GridBase::Ptr grid)
{
	return openvdb::gridPtrCast<GridType>(grid);
}

/* Calls a functor on the fully resolved grid type. */
template <typename GridType, typename GridBaseType, typename OpType>
inline void call_typed_grid(GridBaseType &grid, OpType &op)
{
	op.template operator()<GridType>(VDB_grid_cast<GridType>(grid));
}

#define PROCESS_TYPED_GRID(GridBase) \
	template <typename Op> \
	inline bool process_typed_grid(GridBase grid, int storage, Op &op) \
	{ \
		switch (storage) { \
			case GRID_STORAGE_FLOAT: \
				call_typed_grid<openvdb::FloatGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_DOUBLE: \
				call_typed_grid<openvdb::DoubleGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_BOOL: \
				call_typed_grid<openvdb::BoolGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_INT32: \
				call_typed_grid<openvdb::Int32Grid>(grid, op); \
				return true; \
			case GRID_STORAGE_INT64: \
				call_typed_grid<openvdb::Int64Grid>(grid, op); \
				return true; \
			case GRID_STORAGE_VEC3I: \
				call_typed_grid<openvdb::Vec3IGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_VEC3S: \
				call_typed_grid<openvdb::Vec3SGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_VEC3D: \
				call_typed_grid<openvdb::Vec3DGrid>(grid, op); \
				return true; \
		} \
		return false; \
	} \
	template <typename Op> \
	inline bool process_grid_real(GridBase grid, int storage, Op &op) \
	{ \
		switch (storage) { \
			case GRID_STORAGE_FLOAT: \
				call_typed_grid<openvdb::FloatGrid>(grid, op); \
				return true; \
			case GRID_STORAGE_DOUBLE: \
				call_typed_grid<openvdb::DoubleGrid>(grid, op); \
				return true; \
		} \
		return false; \
	}

PROCESS_TYPED_GRID(const openvdb::GridBase &)
PROCESS_TYPED_GRID(const openvdb::GridBase *)
PROCESS_TYPED_GRID(openvdb::GridBase::ConstPtr)
PROCESS_TYPED_GRID(openvdb::GridBase &)
PROCESS_TYPED_GRID(openvdb::GridBase *)
PROCESS_TYPED_GRID(openvdb::GridBase::Ptr)
