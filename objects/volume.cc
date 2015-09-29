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

#include "volume.h"

#include "render/GPUBuffer.h"
#include "render/GPUTexture.h"

#include "util/util_opengl.h"
#include "util/util_openvdb.h"
#include "util/utils.h"

#include "cube.h"
#include "treetopology.h"

Volume::Volume()
    : m_buffer_data(std::unique_ptr<GPUBuffer>(new GPUBuffer))
    , m_volume_texture(nullptr)
    , m_transfer_texture(nullptr)
    , m_bbox(nullptr)
    , m_topology(nullptr)
    , m_min(glm::vec3(0.0f))
    , m_max(glm::vec3(0.0f))
    , m_size(glm::vec3(0.0f))
    , m_inv_size(glm::vec3(0.0f))
    , m_num_slices(256)
    , m_texture_slices(m_num_slices * 4)
    , m_axis(-1)
    , m_scale(0.0f)
    , m_use_lut(false)
    , m_draw_bbox(false)
    , m_draw_topology(false)
{}

Volume::Volume(openvdb::FloatGrid::Ptr &grid)
    : Volume()
{
	using namespace openvdb;
	using namespace openvdb::math;

	CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
	Coord bbox_min = bbox.min();
	Coord bbox_max = bbox.max();

	/* Get resolution */
	auto extent = bbox_max - bbox_min;
	const int X_DIM = extent[0];
	const int Y_DIM = extent[1];
	const int Z_DIM = extent[2];

	/* Compute grid size */
	BBoxd ws_bbox = grid->transform().indexToWorld(bbox);
	Vec3f min = ws_bbox.min(); // grid->transform().indexToWorld(bbox_min);
	Vec3f max = ws_bbox.max(); // grid->transform().indexToWorld(bbox_max);

	m_min = convertOpenVDBVec(min);
	m_max = convertOpenVDBVec(max);

	m_size = (m_max - m_min);
	m_inv_size = 1.0f / m_size;

	m_bbox = std::unique_ptr<Cube>(new Cube(m_min, m_max));
	m_topology = std::unique_ptr<TreeTopology>(new TreeTopology(grid));

#if 0
	printf("Dimensions: %d, %d, %d\n", X_DIM, Y_DIM, Z_DIM);
	printf("Min: %f, %f, %f\n", min[0], min[1], min[2]);
	printf("Max: %f, %f, %f\n", max[0], max[1], max[2]);
	printf("Bbox Min: %d, %d, %d\n", bbox_min[0], bbox_min[1], bbox_min[2]);
	printf("Bbox Max: %d, %d, %d\n", bbox_max[0], bbox_max[1], bbox_max[2]);
#endif

	/* Copy data */
	GLfloat *data = new GLfloat[X_DIM * Y_DIM * Z_DIM];

	convert_grid(*grid, data, bbox_min, bbox_max, m_scale);

	m_volume_texture = new GPUTexture(GL_TEXTURE_3D, 0);
	m_volume_texture->bind();
	m_volume_texture->setType(GL_FLOAT, GL_RED, GL_RED);
	m_volume_texture->setMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_volume_texture->setWrapping(GL_CLAMP_TO_BORDER);
	m_volume_texture->create(data, extent.asPointer());
	m_volume_texture->generateMipMap(0, 4);
	m_volume_texture->unbind();
	gl_check_errors();

	delete [] data;

	loadTransferFunction();
	loadVolumeShader();
}

Volume::~Volume()
{
	delete m_volume_texture;
	delete m_transfer_texture;
}

void Volume::loadVolumeShader()
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
		glUniform1i(m_shader("lut"), m_transfer_texture->unit());

		glUniform3fv(m_shader("offset"), 1, &m_min[0]);
		glUniform3fv(m_shader("inv_size"), 1, &m_inv_size[0]);
		glUniform1f(m_shader("scale"), m_scale);
	}
	m_shader.unUse();

	const auto &vsize = MAX_SLICES * 4 * sizeof(glm::vec3);
	const auto &isize = MAX_SLICES * 6 * sizeof(GLuint);

	m_buffer_data->bind();
	m_buffer_data->create_vertex_buffer(nullptr, vsize);
	m_buffer_data->create_index_buffer(nullptr, isize);
	m_buffer_data->attrib_pointer(m_shader["vertex"], 3);
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

	m_transfer_texture = new GPUTexture(GL_TEXTURE_1D, 1);
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
	auto count = 0;
	auto depth = m_min[m_axis];
	auto slice_size = m_size[m_axis] / m_num_slices;

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

	for (auto slice(0); slice < m_num_slices; slice++) {
		glm::vec3 v0 = vertices[m_axis][0];
		glm::vec3 v1 = vertices[m_axis][1];
		glm::vec3 v2 = vertices[m_axis][2];
		glm::vec3 v3 = vertices[m_axis][3];

		v0[m_axis] = depth;
		v1[m_axis] = depth;
		v2[m_axis] = depth;
		v3[m_axis] = depth;

		m_texture_slices[count++] = v0;
		m_texture_slices[count++] = v1;
		m_texture_slices[count++] = v2;
		m_texture_slices[count++] = v3;

		indices[idx_count++] = idx + 0;
		indices[idx_count++] = idx + 1;
		indices[idx_count++] = idx + 2;
		indices[idx_count++] = idx + 0;
		indices[idx_count++] = idx + 2;
		indices[idx_count++] = idx + 3;

		depth += slice_size;
		idx += 4;
	}

	m_buffer_data->update_vertex_buffer(&(m_texture_slices[0].x), m_texture_slices.size() * sizeof(glm::vec3));
	m_buffer_data->update_index_buffer(indices, idx_count * sizeof(GLuint));

	delete [] indices;
}

void Volume::render(const glm::vec3 &dir, const glm::mat4 &MVP)
{
	slice(dir);

	if (m_draw_bbox) {
		m_bbox->render(MVP);
	}

	if (m_draw_topology) {
		m_topology->render(MVP);
	}

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_shader.use();
	{
		m_buffer_data->bind();
		m_volume_texture->bind();
		m_transfer_texture->bind();

		glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform1i(m_shader("use_lut"), m_use_lut);
		glDrawElements(GL_TRIANGLES, m_num_slices * 6, GL_UNSIGNED_INT, nullptr);

		m_transfer_texture->unbind();
		m_volume_texture->unbind();
		m_buffer_data->unbind();
	}
	m_shader.unUse();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void Volume::changeNumSlicesBy(int x)
{
	m_num_slices += x;
	m_num_slices = std::min(MAX_SLICES, std::max(m_num_slices, 3));
	m_texture_slices.resize(m_num_slices * 4);
}

void Volume::toggleUseLUT()
{
	m_use_lut = !m_use_lut;
}

void Volume::toggleBBoxDrawing()
{
	m_draw_bbox = !m_draw_bbox;
}

void Volume::toggleTopologyDrawing()
{
	m_draw_topology = !m_draw_topology;
}
