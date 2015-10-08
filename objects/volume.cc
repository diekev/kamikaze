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

#include <glm/gtc/type_ptr.hpp>

#include <openvdb/openvdb.h>
#include <openvdb/tools/Dense.h>

#include "volume.h"

#include "render/GPUBuffer.h"

#include "util/util_opengl.h"
#include "util/util_openvdb.h"
#include "util/utils.h"

#include "cube.h"
#include "treetopology.h"

using openvdb::math::Coord;

void max_leaf_per_axis(const int dim[3], int voxel_per_leaf, int num_leaf, int result[3])
{
	result[0] = (dim[0] - (dim[0] % voxel_per_leaf)) / voxel_per_leaf;
	result[1] = (dim[1] - (dim[1] % voxel_per_leaf)) / voxel_per_leaf;
	result[2] = num_leaf / (result[0] * result[1]) + 1;
}

void texture_from_leaf(const openvdb::FloatGrid &grid, GPUTexture &texture, GPUTexture &index_texture)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;
	typedef FloatTree::LeafCIter LeafCIterType;
	typedef FloatGrid::ValueType ValueType;

	const int DIM = LeafType::DIM;
	const int LOG2DIM = LeafType::LOG2DIM;
	const int NUM_VOXELS = LeafType::NUM_VOXELS;

	/* compute number of leaves per axis there will be in the packed texture */

	CoordBBox bbox;
	grid.tree().evalLeafBoundingBox(bbox);
	auto dim = bbox.dim();

	int leaf_count = grid.tree().leafCount();

	Coord index_volume_res(dim >> LOG2DIM);

	tools::Dense<Vec3s> index_volume(index_volume_res, bbox.min() >> LOG2DIM);
	index_volume.fill(Vec3s(-1.0f));

	Vec3i leaf_per_axis;
	max_leaf_per_axis(dim.asPointer(), DIM, leaf_count, leaf_per_axis.asPointer());

	Vec3i packed_volume_res(leaf_per_axis * DIM);

	texture.bind();
	texture.setType(GL_FLOAT, GL_RED, GL_RED);
	texture.setMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	texture.setWrapping(GL_CLAMP_TO_BORDER);
	texture.create(nullptr, packed_volume_res.asPointer());
	texture.generateMipMap(0, 4);
	gl_check_errors();

	Vec3i offset(0);
	FloatGrid::ConstAccessor acc = grid.getConstAccessor();
	ValueType *data = new ValueType[NUM_VOXELS];

	int leaf_size[3] = { DIM, DIM, DIM };

	for (LeafCIterType leaf_iter = grid.tree().cbeginLeaf(); leaf_iter; ++leaf_iter) {
		const LeafType &leaf = *leaf_iter.getLeaf();
		const Coord &lco = leaf.origin();
		Coord ijk;
		int &x = ijk[0], &y = ijk[1], &z = ijk[2];

		/* VDB is using zyx storage so we transform the leaf voxels to xyz storage */
		int index(0);
		for (z = lco[2]; z < lco[2] + DIM; ++z) {
			for (y = lco[1]; y < lco[1] + DIM; ++y) {
				for (x = lco[0]; x < lco[0] + DIM; ++x, ++index) {
					data[index] = acc.getValue(ijk);
				}
			}
		}

		texture.createSubImage(data, leaf_size, offset.asPointer());

		gl_check_errors();

		const Coord &co = leaf.origin() >> LOG2DIM;
		const Vec3s value(float(offset[0]) / packed_volume_res[0],
		                  float(offset[1]) / packed_volume_res[1],
                          float(offset[2]) / packed_volume_res[2]);

		index_volume.setValue(co, value);

		offset[0] += DIM;

		if (offset[0] == packed_volume_res[0]) {
			offset[0] = 0;
			offset[1] += DIM;

			if (offset[1] == packed_volume_res[1]) {
				offset[1] = 0;
				offset[2] += DIM;
				assert(offset[2] <= packed_volume_res[2]);
			}
		}
	}

	texture.unbind();

	index_texture.bind();
	index_texture.setType(GL_FLOAT, GL_RGB, GL_RGB);
	index_texture.setMinMagFilter(GL_LINEAR, GL_LINEAR);
	index_texture.setWrapping(GL_CLAMP_TO_BORDER);
	index_texture.create(&index_volume.data()[0][0], index_volume_res.asPointer());
	index_texture.unbind();
	gl_check_errors();

	delete [] data;
}

Volume::Volume(openvdb::FloatGrid::Ptr &grid)
    : VolumeBase(grid)
    , m_volume_texture(nullptr)
    , m_transfer_texture(nullptr)
    , m_index_texture(nullptr)
    , m_num_slices(256)
    , m_axis(-1)
    , m_value_scale(1.0f)
    , m_use_lut(false)
{
	using namespace openvdb;
	using namespace openvdb::math;

	m_draw_type = GL_TRIANGLES;

	m_volume_texture = std::unique_ptr<GPUTexture>(new GPUTexture(GL_TEXTURE_3D, 0));
	m_transfer_texture = std::unique_ptr<GPUTexture>(new GPUTexture(GL_TEXTURE_1D, 1));
	m_index_texture = std::unique_ptr<GPUTexture>(new GPUTexture(GL_TEXTURE_3D, 2));

	texture_from_leaf(*grid, *m_volume_texture, *m_index_texture);

	loadTransferFunction();
	loadVolumeShader();
}

void Volume::loadVolumeShader()
{
	m_program.loadFromFile(GL_VERTEX_SHADER, "shader/texture_slicer.vert");
	m_program.loadFromFile(GL_FRAGMENT_SHADER, "shader/texture_slicer.frag");

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addUniform("MVP");
		m_program.addUniform("offset");
		m_program.addUniform("volume");
		m_program.addUniform("lut");
		m_program.addUniform("index_volume");
		m_program.addUniform("use_lut");
		m_program.addUniform("scale");
		m_program.addUniform("inv_size");

		glUniform1i(m_program("volume"), 0);
		glUniform1i(m_program("lut"), m_transfer_texture->unit());
		glUniform1i(m_program("index_volume"), m_index_texture->unit());

		glUniform3fv(m_program("offset"), 1, &m_min[0]);
		glUniform3fv(m_program("inv_size"), 1, &m_inv_size[0]);
		glUniform1f(m_program("scale"), m_value_scale);
	}
	m_program.disable();

	const auto &vsize = MAX_SLICES * 4 * sizeof(glm::vec3);
	const auto &isize = MAX_SLICES * 6 * sizeof(GLuint);

	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(nullptr, vsize);
	m_buffer_data->generateIndexBuffer(nullptr, isize);
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->unbind();
}

void Volume::loadTransferFunction()
{
	/* transfer function (lookup table) color values */
	const glm::vec3 jet_values[12] = {
	    glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.5f),
		glm::vec3(1.0f, 0.0f, 1.0f),

		glm::vec3(0.5f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.5f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),

		glm::vec3(0.0f, 1.0f, 0.5f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.5f, 1.0f, 0.0f),

		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.5f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
	};

	int size = 256;
	float data[size][3];
	int indices[12];

	for (int i = 0; i < 12; ++i) {
		indices[i] = i * 21;
	}

	/* for each adjacent pair of colors, find the difference in the RGBA values
	 * and then interpolate */
	for (int j = 0; j < 12 - 1; ++j) {
		auto color_diff = jet_values[j + 1] - jet_values[j];
		auto index = indices[j + 1] - indices[j];
		auto inc = color_diff / static_cast<float>(index);

		for (int i = indices[j] + 1; i < indices[j + 1]; ++i) {
			data[i][0] = jet_values[j].r + i * inc.r;
			data[i][1] = jet_values[j].g + i * inc.g;
			data[i][2] = jet_values[j].b + i * inc.b;
		}
	}

	m_transfer_texture->bind();
	m_transfer_texture->setType(GL_FLOAT, GL_RGB, GL_RGB);
	m_transfer_texture->setMinMagFilter(GL_LINEAR, GL_LINEAR);
	m_transfer_texture->setWrapping(GL_REPEAT);
	m_transfer_texture->create(&data[0][0], &size);
	m_transfer_texture->unbind();
}

void Volume::slice(const glm::vec3 &view_dir)
{
	auto axis = axis_dominant_v3_single(glm::value_ptr(view_dir));

	if (m_axis == axis) {
		return;
	}

	m_axis = axis;
	auto depth = m_min[m_axis];
	auto slice_size = m_dimensions[m_axis] / m_num_slices;

	/* always process slices in back to front order! */
	if (view_dir[m_axis] > 0.0f) {
		depth = m_max[m_axis];
		slice_size = -slice_size;
	}

	const glm::vec3 vertices[3][4] = {
	    {
	        glm::vec3(0.0f, m_min[1], m_min[2]),
	        glm::vec3(0.0f, m_max[1], m_min[2]),
	        glm::vec3(0.0f, m_max[1], m_max[2]),
	        glm::vec3(0.0f, m_min[1], m_max[2])
	    },
	    {
	        glm::vec3(m_min[0], 0.0f, m_min[2]),
	        glm::vec3(m_min[0], 0.0f, m_max[2]),
	        glm::vec3(m_max[0], 0.0f, m_max[2]),
	        glm::vec3(m_max[0], 0.0f, m_min[2])
	    },
	    {
	        glm::vec3(m_min[0], m_min[1], 0.0f),
	        glm::vec3(m_min[0], m_max[1], 0.0f),
	        glm::vec3(m_max[0], m_max[1], 0.0f),
	        glm::vec3(m_max[0], m_min[1], 0.0f)
	    }
	};

	GLuint *indices = new GLuint[m_num_slices * 6];
	int idx = 0, idx_count = 0;

	m_vertices.reserve(m_num_slices * 4);

	for (auto slice(0); slice < m_num_slices; slice++) {
		glm::vec3 v0 = vertices[m_axis][0];
		glm::vec3 v1 = vertices[m_axis][1];
		glm::vec3 v2 = vertices[m_axis][2];
		glm::vec3 v3 = vertices[m_axis][3];

		v0[m_axis] = depth;
		v1[m_axis] = depth;
		v2[m_axis] = depth;
		v3[m_axis] = depth;

		m_vertices.push_back(v0);
		m_vertices.push_back(v1);
		m_vertices.push_back(v2);
		m_vertices.push_back(v3);

		indices[idx_count++] = idx + 0;
		indices[idx_count++] = idx + 1;
		indices[idx_count++] = idx + 2;
		indices[idx_count++] = idx + 0;
		indices[idx_count++] = idx + 2;
		indices[idx_count++] = idx + 3;

		depth += slice_size;
		idx += 4;
	}

	m_buffer_data->updateVertexBuffer(&m_vertices[0][0], m_vertices.size() * sizeof(glm::vec3));
	m_buffer_data->updateIndexBuffer(indices, idx_count * sizeof(GLuint));

	delete [] indices;
}

void Volume::render(const glm::mat4 &MVP, const glm::mat3 &N, const glm::vec3 &dir)
{
	if (m_need_update) {
		updateMatrix();
		updateGridTransform();
		m_need_update = false;
	}

	slice(dir);

	if (m_draw_bbox) {
		m_bbox->render(MVP, N, dir);
	}

	if (m_draw_topology) {
		m_topology->render(MVP);
	}

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();
		m_volume_texture->bind();
		m_transfer_texture->bind();
		m_index_texture->bind();

		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform1i(m_program("use_lut"), m_use_lut);
		glDrawElements(m_draw_type, m_num_slices * 6, GL_UNSIGNED_INT, nullptr);

		m_index_texture->unbind();
		m_transfer_texture->unbind();
		m_volume_texture->unbind();
		m_buffer_data->unbind();
		m_program.disable();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void Volume::changeNumSlicesBy(int x)
{
	m_num_slices += x;
	m_num_slices = std::min(MAX_SLICES, std::max(m_num_slices, 3));
	m_vertices.resize(m_num_slices * 4);
}

void Volume::toggleUseLUT()
{
	m_use_lut = !m_use_lut;
}
