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
#include <openvdb/tools/Interpolation.h>

using openvdb::ScalarGrid;
using openvdb::VectorGrid;

using openvdb::math::Vec3s;

/* ******************************** Accessors ******************************** */

using ScalarAccessor = ScalarGrid::Accessor;
using VectorAccessor = VectorGrid::Accessor;
using ConstScalarAccessor = ScalarGrid::ConstAccessor;
using ConstVectorAccessor = VectorGrid::ConstAccessor;

/* ********************************* Samplers ******************************** */

using PointScalarSampler  = openvdb::tools::GridSampler<ConstScalarAccessor, openvdb::tools::PointSampler>;
using PointVectorSampler  = openvdb::tools::GridSampler<ConstVectorAccessor, openvdb::tools::StaggeredPointSampler>;
using LinearScalarSampler = openvdb::tools::GridSampler<ConstScalarAccessor, openvdb::tools::BoxSampler>;
using LinearVectorSampler = openvdb::tools::GridSampler<ConstVectorAccessor, openvdb::tools::StaggeredBoxSampler>;
