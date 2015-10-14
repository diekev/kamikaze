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

#include "types.h"
#include "globals.h"

void add_buoyancy(const float dt,
                  VectorGrid::Ptr &velocity,
                  ScalarGrid::Ptr &density,
                  ScalarGrid::Ptr &temperature,
                  const openvdb::Int32Grid::Ptr &flags);

Vec3s accumulate_forces(const std::vector<Vec3s> &forces);

void set_neumann_boundary(VectorGrid &velocity, const openvdb::Int32Grid::Ptr &flags);

openvdb::Int32Grid::Ptr build_flag_grid(const ScalarGrid::Ptr &inflow, const openvdb::BoolGrid::Ptr &collision);

void solve_pressure(const float dt, VectorGrid &velocity,
                    ScalarGrid &pressure, const openvdb::Int32Grid &flags);
