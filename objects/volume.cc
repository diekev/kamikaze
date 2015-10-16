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

#include "util/util_opengl.h"
#include "util/util_openvdb.h"
#include "util/util_openvdb_process.h"
#include "util/utils.h"

const int MAX_SLICES = 512;

Volume::Volume(openvdb::GridBase::Ptr grid)
    : VolumeBase(grid)
    , m_volume_texture(nullptr)
    , m_transfer_texture(nullptr)
    , m_num_slices(256)
    , m_axis(-1)
    , m_value_scale(0.0f)
    , m_use_lut(false)
    , m_num_textures(0)
{
	using namespace openvdb;
	using namespace openvdb::math;

	m_elements = m_num_slices * 6;

	/* Get resolution & copy data */
	openvdb::math::CoordBBox bbox = m_grid->evalActiveVoxelBoundingBox();

	SparseToDenseOp op;
	op.bbox = bbox;
	op.data = new GLfloat[bbox.volume()];

	process_grid_real(m_grid, get_grid_storage(*m_grid), op);

	m_volume_texture = GPUTexture::create(GL_TEXTURE_3D, m_num_textures++);
	m_volume_texture->bind();
	m_volume_texture->setType(GL_FLOAT, GL_RED, GL_RED);
	m_volume_texture->setMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_volume_texture->setWrapping(GL_CLAMP_TO_BORDER);
	m_volume_texture->createTexture(op.data, bbox.dim().asPointer());
	m_volume_texture->generateMipMap(0, 4);
	m_volume_texture->unbind();
	gl_check_errors();

	loadTransferFunction();
	loadVolumeShader();
}

void Volume::loadVolumeShader()
{
	m_program.loadFromFile(VERTEX_SHADER, "shaders/volume.vert");
	m_program.loadFromFile(FRAGMENT_SHADER, "shaders/volume.frag");

	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addUniform("MVP");
		m_program.addUniform("offset");
		m_program.addUniform("volume");
		m_program.addUniform("lut");
		m_program.addUniform("use_lut");
		m_program.addUniform("scale");
		m_program.addUniform("matrix");

		glUniform1i(m_program("volume"), m_volume_texture->unit());
		glUniform1i(m_program("lut"), m_transfer_texture->unit());
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

	m_transfer_texture = GPUTexture::create(GL_TEXTURE_1D, m_num_textures++);
	m_transfer_texture->bind();
	m_transfer_texture->setType(GL_FLOAT, GL_RGB, GL_RGB);
	m_transfer_texture->setMinMagFilter(GL_LINEAR, GL_LINEAR);
	m_transfer_texture->setWrapping(GL_REPEAT);
	m_transfer_texture->createTexture(&data[0][0], &size);
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

	GLuint *indices = new GLuint[m_elements];
	int idx = 0, idx_count = 0;

	m_vertices.clear();
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

		m_vertices.push_back(v0 * glm::mat3(m_inv_matrix));
		m_vertices.push_back(v1 * glm::mat3(m_inv_matrix));
		m_vertices.push_back(v2 * glm::mat3(m_inv_matrix));
		m_vertices.push_back(v3 * glm::mat3(m_inv_matrix));

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

void Volume::render(const glm::mat4 &MVP, const glm::mat3 &N,
                    const glm::vec3 &dir, const bool for_outline)
{
	slice(dir);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();
		m_volume_texture->bind();
		m_transfer_texture->bind();

		auto min = m_min * glm::mat3(m_inv_matrix);
		glUniform3fv(m_program("offset"), 1, &min[0]);
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(m_matrix));
		glUniform1i(m_program("use_lut"), m_use_lut);
		glDrawElements(m_draw_type, m_elements, GL_UNSIGNED_INT, nullptr);

		m_transfer_texture->unbind();
		m_volume_texture->unbind();
		m_buffer_data->unbind();
		m_program.disable();
	}

	glDisable(GL_BLEND);

	(void)N;
	(void)for_outline;
}

void Volume::numSlices(int x)
{
	m_num_slices = std::min(MAX_SLICES, std::max(x, 3));
	m_vertices.resize(m_num_slices * 4);
	m_elements = m_num_slices * 6;
}

void Volume::useLUT(bool b)
{
	m_use_lut = b;
}
