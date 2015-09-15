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

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define DWREAL_IS_DOUBLE 0
#include <openvdb/openvdb.h>
#include <openvdb/tools/GridTransformer.h>

#include "render/GLSLShader.h"
#include "util/util_opengl.h"
#include "util/util_openvdb.h"
#include "util/utils.h"

#include "cube.h"
#include "volume.h"

#define TEXTURE_ATLAS

const float EPSILON = 0.0001f;

using openvdb::math::Coord;

void max_leaf_per_axis(const int dim[3], int voxel_per_leaf, int num_leaf, int result[3])
{
	result[0] = (dim[0] - (dim[0] % voxel_per_leaf)) / voxel_per_leaf;
	result[1] = (dim[1] - (dim[1] % voxel_per_leaf)) / voxel_per_leaf;
	result[2] = num_leaf / (result[0] * result[1]) + 1;
}

void texture_from_leaf(const openvdb::FloatGrid &grid, GLuint &texture_id)
{
	Timer(__func__);

	using namespace openvdb;
	using namespace openvdb::math;

	typedef FloatTree::LeafNodeType LeafType;
	typedef FloatTree::LeafCIter LeafCIterType;
	typedef FloatGrid::ValueType ValueType;

	const int DIM = LeafType::DIM;
	const int LOG2DIM = LeafType::LOG2DIM;

	/* compute number of leaves per axis there will be in the packed texture */

	Coord bbox_min, bbox_max;
	int leaf_count = evalLeafBBoxAndCount(grid.tree(), bbox_min, bbox_max);
	auto leaf_bbox_extent = bbox_max - bbox_min;

	Vec3i leaf_per_axis;
	max_leaf_per_axis(leaf_bbox_extent.asPointer(), DIM, leaf_count, leaf_per_axis.asPointer());

	Vec3i index_texture_res(
	        leaf_bbox_extent[0] >> LOG2DIM,
	        leaf_bbox_extent[1] >> LOG2DIM,
	        leaf_bbox_extent[2] >> LOG2DIM);

	auto leaf_slab_size = index_texture_res[0] * index_texture_res[1];
	auto index_texture_size = index_texture_res[2] * leaf_slab_size;

#ifndef NDEBUG
	printf("Index Texture Res: %d, %d, %d.\n", index_texture_res.x(), index_texture_res.y(), index_texture_res.z());
#endif

	assert(index_texture_size > 0);

	Vec3i packed_texture_res(leaf_per_axis * 8);
	create_texture_3D(texture_id, packed_texture_res.asPointer(), nullptr);

	std::vector<glm::ivec3> index_texture;
	index_texture.resize(index_texture_size, glm::ivec3(-1));

	GLint xoffset = 0, yoffset = 0, zoffset = 0;

	for (LeafCIterType leaf_iter = grid.tree().cbeginLeaf(); leaf_iter; ++leaf_iter) {
		const LeafType &leaf = *leaf_iter.getLeaf();
		const ValueType *data = leaf.buffer().data();

		glTexSubImage3D(GL_TEXTURE_3D, 0,
		                xoffset, yoffset, zoffset,
		                DIM, DIM, DIM,
		                GL_RED, GL_FLOAT, data);

		gl_check_errors();

		const Coord &co = (leaf.origin() - bbox_min) >> LOG2DIM;
		int index = co.x() + co.y() * index_texture_res[0] + co.z() * leaf_slab_size;

#ifndef NDEBUG
		if (index >= index_texture_size) {
			printf("Index too big: %d, coord: %d, %d, %d, num leaves: %d, tex size: %d\n",
			       index, co.x(), co.y(), co.z(), leaf_count, index_texture_size);
		}

		if (index < 0) {
			printf("Index too small: %d, coord: %d, %d, %d, num leaves: %d, tex size: %d\n",
			       index, co.x(), co.y(), co.z(), leaf_count, index_texture_size);
		}
#endif

		index_texture[index] = glm::ivec3(xoffset, yoffset, zoffset);

		xoffset += DIM;

		if (xoffset == packed_texture_res[0]) {
			xoffset = 0;
			yoffset += DIM;

			if (yoffset == packed_texture_res[1]) {
				yoffset = 0;
				zoffset += DIM;
			}
		}
	}
}

int axis_dominant_v3_single(const glm::vec3 &vec)
{
	const float x = glm::abs(vec[0]);
	const float y = glm::abs(vec[1]);
	const float z = glm::abs(vec[2]);
	return ((x > y) ?
	       ((x > z) ? 0 : 2) :
	       ((y > z) ? 1 : 2));
}

VolumeShader::VolumeShader()
    : m_vao(0)
    , m_vbo(0)
    , m_texture_id(0)
    , m_bbox(nullptr)
    , m_min(glm::vec3(0.0f))
    , m_max(glm::vec3(0.0f))
    , m_size(glm::vec3(0.0f))
    , m_inv_size(glm::vec3(0.0f))
    , m_num_slices(256)
    , m_axis(-1)
    , m_scale(1.0f)
    , m_use_lut(false)
    , m_draw_bbox(false)
{
	m_texture_slices.resize(m_num_slices * 6);
}

VolumeShader::~VolumeShader()
{
	m_shader.deleteShaderProgram();

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);

	glDeleteTextures(1, &m_texture_id);
	glDeleteTextures(1, &m_transfer_func_id);

	delete m_bbox;
}

bool VolumeShader::init(const std::string &filename, std::ostream &os)
{
	if (loadVolumeFile(filename, os)) {
		loadVolumeShader();
		loadTransferFunction();
		return true;
	}

	return false;
}

bool VolumeShader::loadVolumeFile(const std::string &volume_file, std::ostream &os)
{
	using namespace openvdb;
	using namespace openvdb::math;

	initialize();
	io::File file(volume_file);

	if (file.open()) {
		FloatGrid::Ptr grid;

		if (file.hasGrid(Name("Density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("Density")));
		}
		else if (file.hasGrid(Name("density"))) {
			grid = gridPtrCast<FloatGrid>(file.readGrid(Name("density")));
		}
		else {
			os << "No density grid found in file: \'" << volume_file << "\'!\n";
			return false;
		}

		if (grid->getGridClass() == GRID_LEVEL_SET) {
			os << "Grid \'" << grid->getName() << "\'is a level set!\n";
			return false;
		}

		auto meta_map = file.getMetadata();

		file.close();

		if ((*meta_map)["creator"]) {
			auto creator = (*meta_map)["creator"]->str();

			/* If the grid comes from Blender (Z-up), rotate it so it is Y-up */
			if (creator == "Blender/OpenVDBWriter") {
				Timer("Transform Blender Grid");

				Mat4R rotate_mat(Mat4R::identity());
				rotate_mat.preRotate(X_AXIS, -M_PI_2);

				/* make sure the new grid has the same transform and metadatas
				 * as the old. */
				FloatGrid::Ptr xformed_grid = grid->copy(CopyPolicy::CP_NEW);

				tools::GridTransformer transformer(rotate_mat);
				transformer.transformGrid<tools::PointSampler>(*grid, *xformed_grid);
				tools::prune(xformed_grid->tree());

				grid = xformed_grid;
			}
		}

		CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
		Coord bbox_min = bbox.min();
		Coord bbox_max = bbox.max();

		/* Compute grid size */
		Vec3f min = grid->transform().indexToWorld(bbox_min);
		Vec3f max = grid->transform().indexToWorld(bbox_max);

		for (int i(0); i < 3; ++i) {
			m_min[i] = min[i];
			m_max[i] = max[i];
		}

		m_size = (m_max - m_min);
		m_inv_size = 1.0f / m_size;

		m_bbox = new Cube(m_min, m_max);

#ifdef TEXTURE_ATLAS
		texture_from_leaf(*grid, m_texture_id);
#else
		/* Get resolution */
		auto extent = bbox_max - bbox_min;
		GLfloat *data = new GLfloat[extent[0] * extent[1] * extent[2]];

		convert_grid(*grid, data, bbox_min, bbox_max, m_scale);
		create_texture_3D(m_texture_id, extent.asPointer(), data);

		delete [] data;
#endif

#if 0
		printf("Dimensions: %d, %d, %d\n", X_DIM, Y_DIM, Z_DIM);
		printf("Min: %f, %f, %f\n", min[0], min[1], min[2]);
		printf("Max: %f, %f, %f\n", max[0], max[1], max[2]);
#endif
		return true;
	}

	os << "Unable to open file \'" << volume_file << "\'\n";

	return false;
}

void VolumeShader::loadVolumeShader()
{
	m_shader.loadFromFile(GL_VERTEX_SHADER, "shader/texture_slicer.vert");
	m_shader.loadFromFile(GL_FRAGMENT_SHADER, "shader/texture_slicer.frag");

	m_shader.createAndLinkProgram();

	m_shader.use();
	{
		m_shader.addAttribute("vertex");
		m_shader.addUniform("MVP");
		m_shader.addUniform("offset");
		m_shader.addUniform("volume");
		m_shader.addUniform("lut");
		m_shader.addUniform("use_lut");
		m_shader.addUniform("scale");
		m_shader.addUniform("inv_size");

		glUniform1i(m_shader("volume"), 0);
		glUniform1i(m_shader("lut"), 1);

		glUniform3fv(m_shader("offset"), 1, &m_min[0]);
		glUniform3fv(m_shader("inv_size"), 1, &m_inv_size[0]);
		glUniform1f(m_shader("scale"), m_scale);
	}
	m_shader.unUse();

	/* setup the vertex array and buffer objects */
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	/* pass the sliced volume vector to buffer output memory */
	glBufferData(GL_ARRAY_BUFFER, MAX_SLICES * 6 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	/* enable vertex attribute array for position */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);
}

void VolumeShader::loadTransferFunction()
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

	float data[256][3];
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

	create_texture_1D(m_transfer_func_id, 256, &data[0][0]);
}

void VolumeShader::slice(const glm::vec3 &view_dir)
{
	auto axis = axis_dominant_v3_single(view_dir);

	if (m_axis == axis) {
		return;
	}

	m_axis = axis;
	auto count = 0;
	auto depth = m_min[m_axis];
	auto slice_size = m_size[m_axis] / m_num_slices;

	/* always process slices in back to front order! */
	if (view_dir[m_axis] > 0.0f) {
		depth = m_max[m_axis];
		slice_size = -slice_size;
	}

	for (auto slice(0); slice < m_num_slices; slice++) {
		const glm::vec3 vertices[3][4] = {
		    {
		        glm::vec3(depth, m_min[1], m_min[2]),
		        glm::vec3(depth, m_max[1], m_min[2]),
		        glm::vec3(depth, m_max[1], m_max[2]),
		        glm::vec3(depth, m_min[1], m_max[2])
		    },
		    {
		        glm::vec3(m_min[0], depth, m_min[2]),
		        glm::vec3(m_min[0], depth, m_max[2]),
		        glm::vec3(m_max[0], depth, m_max[2]),
		        glm::vec3(m_max[0], depth, m_min[2])
		    },
		    {
		        glm::vec3(m_min[0], m_min[1], depth),
		        glm::vec3(m_min[0], m_max[1], depth),
		        glm::vec3(m_max[0], m_max[1], depth),
		        glm::vec3(m_max[0], m_min[1], depth)
		    }
		};

		m_texture_slices[count++] = vertices[m_axis][0];
		m_texture_slices[count++] = vertices[m_axis][1];
		m_texture_slices[count++] = vertices[m_axis][2];
		m_texture_slices[count++] = vertices[m_axis][0];
		m_texture_slices[count++] = vertices[m_axis][2];
		m_texture_slices[count++] = vertices[m_axis][3];

		depth += slice_size;
	}

	/* update buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_texture_slices.size() * sizeof(glm::vec3), &(m_texture_slices[0].x));
}

void VolumeShader::render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated)
{
	if (is_rotated) {
		slice(dir);
	}

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(m_vao);

	m_shader.use();
	{
		glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform1i(m_shader("use_lut"), m_use_lut);
		glDrawArrays(GL_TRIANGLES, 0, m_texture_slices.size());
	}
	m_shader.unUse();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (m_draw_bbox) {
		m_bbox->render(MVP);
	}
}

void VolumeShader::changeNumSlicesBy(int x)
{
	m_num_slices += x;
	m_num_slices = std::min(MAX_SLICES, std::max(m_num_slices, 3));
	m_texture_slices.resize(m_num_slices * 6);
}

void VolumeShader::toggleUseLUT()
{
	m_use_lut = ((m_use_lut) ? false : true);
}

void VolumeShader::toggleBBoxDrawing()
{
	m_draw_bbox = ((m_draw_bbox) ? false : true);
}
