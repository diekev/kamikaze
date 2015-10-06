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

#include <glm/glm.hpp>
#include <openvdb/openvdb.h>

#include "object.h"

#include "render/GPUTexture.h"
#include "util/util_render.h"

const int MAX_SLICES = 512;

class Cube;
class GPUBuffer;
class TreeTopology;

class Volume : public Object {
	std::unique_ptr<GPUTexture> m_volume_texture, m_transfer_texture, m_index_texture;

	std::unique_ptr<Cube> m_bbox;
	std::unique_ptr<TreeTopology> m_topology;

	int m_num_slices;
	std::vector<glm::vec3> m_texture_slices;

	int m_axis;
	float m_scale; // scale of the values contained in the grid (1 / (max - min))
	bool m_use_lut;

	void loadTransferFunction();
	void loadVolumeShader();

public:
	Volume();
	Volume(openvdb::FloatGrid::Ptr &grid);
	~Volume() = default;

	void slice(const glm::vec3 &view_dir);
	void render(const glm::mat4 &MVP, const glm::mat3 &N, const glm::vec3 &dir);
	void changeNumSlicesBy(int x);

	void toggleUseLUT();
};
