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
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/util/PagedArray.h>

#include <glm/glm.hpp>

//#include "util/utils.h"  /* for ScopeTimer */

openvdb::FloatGrid::Ptr transform_grid(const openvdb::FloatGrid &grid,
                                       const openvdb::Vec3s &rot,
                                       const openvdb::Vec3s &scale,
                                       const openvdb::Vec3s &translate,
                                       const openvdb::Vec3s &pivot);

struct SparseToDenseOp {
	float *data;
	float scale;
	openvdb::math::CoordBBox bbox;

	~SparseToDenseOp()
	{
		delete [] data;
	}

	template <typename GridType>
	void operator()(typename GridType::ConstPtr grid)
	{
		//Timer("SparseToDenseOp");

		using namespace openvdb;

		typedef typename GridType::ConstAccessor AccessorType;

		AccessorType main_acc = grid->getAccessor();

		const auto &dim = bbox.dim();
		const auto &slabsize = dim[0] * dim[1];

		util::PagedArray<float> min_array, max_array;

		tbb::parallel_for(tbb::blocked_range<int>(bbox.min()[2], bbox.max()[2]),
		        [&](const tbb::blocked_range<int> &r)
		{
			AccessorType acc(main_acc);
			math::Coord ijk;
			int &x = ijk[0], &y = ijk[1], &z = ijk[2];
			z = r.begin();

			auto min_value = std::numeric_limits<float>::max();
			auto max_value = std::numeric_limits<float>::min();

			/* Subtract min z coord so that 'index' always start at zero or above. */
			auto index = (z - bbox.min()[2]) * slabsize;

			for (auto e = r.end(); z <= e; ++z) {
				for (y = bbox.min()[1]; y <= bbox.max()[1]; ++y) {
					for (x = bbox.min()[0]; x <= bbox.max()[0]; ++x, ++index) {
						auto value = acc.getValue(ijk);

						if (value < min_value) {
							min_value = value;
						}
						else if (value > max_value) {
							max_value = value;
						}

						data[index] = value;
					}
				}
			}

			min_array.push_back(min_value);
			max_array.push_back(max_value);
		});

		auto min_value = std::min_element(min_array.begin(), min_array.end());
		auto max_value = std::max_element(max_array.begin(), max_array.end());
		scale = 1.0f / (*max_value - *min_value);
	}
};

struct VolumeMesherOp {
	std::vector<glm::vec3> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	glm::mat4 inv_mat;

	template <typename GridType>
	void operator()(typename GridType::ConstPtr grid)
	{
		using namespace openvdb;

		tools::VolumeToMesh mesher((grid->getGridClass() == GRID_LEVEL_SET) ? 0.0 : 0.001);
		mesher(*grid);

		/* Copy points and generate point normals. */
		vertices.reserve(mesher.pointListSize());
		normals.resize(mesher.pointListSize() * 3);

		for (Index64 n = 0, N = mesher.pointListSize(); n < N; ++n) {
			const math::Vec3s &p = mesher.pointList()[n];
			vertices.push_back(glm::vec3(p[0], p[1], p[2]) * glm::mat3(inv_mat));
		}

		/* Copy primitives */
		tools::PolygonPoolList &polygonPoolList = mesher.polygonPoolList();
		Index64 numQuads = 0;

		for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
			numQuads += polygonPoolList[n].numQuads();
		}

		indices.reserve(numQuads * 6);
		math::Vec3d normal, e1, e2;

		for (Index64 n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
			const tools::PolygonPool &polygons = polygonPoolList[n];

			for (Index64 i = 0, I = polygons.numQuads(); i < I; ++i) {
				const Vec4I &quad = polygons.quad(i);
				indices.push_back(quad[0]);
				indices.push_back(quad[1]);
				indices.push_back(quad[2]);
				indices.push_back(quad[0]);
				indices.push_back(quad[2]);
				indices.push_back(quad[3]);

				e1 = mesher.pointList()[quad[1]];
				e1 -= mesher.pointList()[quad[0]];
				e2 = mesher.pointList()[quad[2]];
				e2 -= mesher.pointList()[quad[1]];
				normal = e1.cross(e2);

				const double length = normal.length();
				if (length > 1.0e-7) normal *= (1.0 / length);

				for (int v = 0; v < 4; ++v) {
					normals[quad[v] * 3]     = static_cast<float>(-normal[0]);
					normals[quad[v] * 3 + 1] = static_cast<float>(-normal[1]);
					normals[quad[v] * 3 + 2] = static_cast<float>(-normal[2]);
				}
			}
		}
	}
};

int axis_dominant_v3_single(const float vec[3]);

/* Functions to convert between glm and openvdb types. */

template <typename T>
glm::vec3 convertOpenVDBVec(const openvdb::math::Vec3<T> &vec)
{
	return glm::vec3(vec[0], vec[1], vec[2]);
}

openvdb::math::Vec3d convertGLMVec(const glm::vec3 &vec);

template <typename T>
glm::mat4 convertOpenVDBMat4(const openvdb::math::Mat4<T> &mat)
{
	glm::mat4 ret;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			ret[i][j] = mat[i][j];
		}
	}

	return ret;
}

template <typename T>
void print_mat4(const openvdb::math::Mat4<T> &mat, const std::string &title = "")
{
	if (!title.empty()) {
		printf("%s:\n", title.c_str());
	}

	for (int i = 0; i < 4; ++i) {
		printf("[%.6f, %.6f, %.6f, %.6f]\n", mat[i][0], mat[i][1], mat[i][2], mat[i][3]);
	}

	printf("\n");
}
