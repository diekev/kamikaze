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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <openvdb/tools/VolumeToMesh.h>

#include "levelset.h"

#include "sculpt/brush.h"

#include "util/utils.h"
#include "util/util_opengl.h"
#include "util/util_openvdb.h"
#include "util/util_openvdb_process.h"

LevelSet::LevelSet(openvdb::GridBase::Ptr grid)
    : VolumeBase(grid)
    , m_isector(nullptr)
{
	loadShader();
	generateMesh(false);
}

void LevelSet::loadShader()
{
	m_program.loadFromFile(GL_VERTEX_SHADER, "shaders/object.vert");
	m_program.loadFromFile(GL_FRAGMENT_SHADER, "shaders/object.frag");
	m_program.createAndLinkProgram();

	m_program.enable();
	{
		m_program.addAttribute("vertex");
		m_program.addAttribute("normal");
		m_program.addUniform("matrix");
		m_program.addUniform("MVP");
		m_program.addUniform("N");
		m_program.addUniform("for_outline");
	}
	m_program.disable();
}

void LevelSet::render(const glm::mat4 &MVP, const glm::mat3 &N,
                      const glm::vec3 &dir, const bool for_outline)
{
	if (m_program.isValid()) {
		m_program.enable();
		m_buffer_data->bind();

		glUniformMatrix4fv(m_program("matrix"), 1, GL_FALSE, glm::value_ptr(m_matrix));
		glUniformMatrix4fv(m_program("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix3fv(m_program("N"), 1, GL_FALSE, glm::value_ptr(N));
		glUniform1i(m_program("for_outline"), for_outline);
		glDrawElements(GL_TRIANGLES, m_elements, GL_UNSIGNED_INT, nullptr);

		m_buffer_data->unbind();
		m_program.disable();
	}

	(void)dir;
}

void LevelSet::generateMesh(const bool is_sculpt_mode)
{
	VolumeMesherOp op;
	op.inv_mat = m_inv_matrix;

	if (is_sculpt_mode)
		op.template operator()<openvdb::FloatGrid>(m_level_set);
	else
		process_grid_real(m_grid, get_grid_storage(*m_grid), op);

	m_elements = op.indices.size();

	m_buffer_data.reset(new GPUBuffer());
	m_buffer_data->bind();
	m_buffer_data->generateVertexBuffer(&op.vertices[0][0], op.vertices.size() * sizeof(glm::vec3));
	m_buffer_data->generateIndexBuffer(&op.indices[0], m_elements * sizeof(GLuint));
	m_buffer_data->attribPointer(m_program["vertex"], 3);
	m_buffer_data->generateNormalBuffer(&op.normals[0], op.normals.size() * sizeof(GLfloat));
	m_buffer_data->attribPointer(m_program["normal"], 3);
	m_buffer_data->unbind();

	gl_check_errors();
}

bool LevelSet::intersectLS(const Ray &ray, Brush *brush)
{
	using namespace openvdb;

	openvdb::math::Vec3d P(ray.pos.x, ray.pos.y, ray.pos.z);
	openvdb::math::Vec3d D(ray.dir.x, ray.dir.y, ray.dir.z);
	D.normalize();

	ray_t vray(P, D, 1e-5, std::numeric_limits<double>::max());

	openvdb::math::Vec3d position;

	m_isector.reset(new isector_t(*m_level_set));
	if (m_isector->intersectsWS(vray, position)) {
		const float radius = brush->radius();
		const float amount = brush->amount();

		FloatGrid::Accessor accessor = m_level_set->getAccessor();
		math::Coord ijk = m_level_set->transform().worldToIndexNodeCentered(position);
		math::Coord co(ijk);
		int &x = ijk[0], &y = ijk[1], &z = ijk[2];

		if (brush->tool() == BRUSH_TOOL_DRAW) {
			float influence;
			for (x = co[0] - radius; x < co[0] + radius; ++x) {
				for (y = co[1] - radius; y < co[1] + radius; ++y) {
					for (z = co[2] - radius; z < co[2] + radius; ++z) {
						influence = brush->influence(co, ijk);

						if (influence > 0.0f) {
							//accessor.modifyValue(ijk, addOp);
							float value = accessor.getValue(ijk);
							accessor.setValue(ijk, value - amount * influence);
						}
					}
				}
			}
		}
		else {
			const float dx = m_level_set->transform().voxelSize()[0];
			const float dt = math::Pow2(dx) / 6.0f;
		    math::GradStencil<FloatGrid> stencil(*m_level_set, dx);

			int diameter = radius + radius;
			float *buffer = new float[diameter * diameter * diameter];

			float influence;
			int index = 0;
			for (x = co[0] - radius; x < co[0] + radius; ++x) {
				for (y = co[1] - radius; y < co[1] + radius; ++y) {
					for (z = co[2] - radius; z < co[2] + radius; ++z, ++index) {
						float value = accessor.getValue(ijk);
						influence = brush->influence(co, ijk);

						if (influence > 0.0f) {
							stencil.moveTo(ijk);
							accessor.setValue(ijk, value - amount * influence);
							const float phi = value + dt * stencil.laplacian();
		                    buffer[index] = phi - amount * influence;
						}
						else {
							buffer[index] = value;
						}
					}
				}
			}

			index = 0;
			/* copy buffer back to voxels */
			for (x = co[0] - radius; x < co[0] + radius; ++x) {
				for (y = co[1] - radius; y < co[1] + radius; ++y) {
					for (z = co[2] - radius; z < co[2] + radius; ++z, ++index) {
						accessor.setValue(ijk, buffer[index]);
					}
				}
			}

			delete [] buffer;
		}

		m_topology_changed = true;

		if (m_draw_topology) {
			m_topology.reset(new TreeTopology(m_level_set));
		}

		generateMesh(true);
	}

	return false;
}

void LevelSet::swapGrids(const bool is_scuplt_mode)
{
	if (is_scuplt_mode) {
		m_level_set = openvdb::gridPtrCast<openvdb::FloatGrid>(m_grid);
		m_topology_changed = true;  /* force update for ray intersection */
	}
	else {
		m_grid = m_level_set;
	}
}
