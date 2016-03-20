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

#include <kamikaze/object.h>

#include <openvdb/openvdb.h>

class TreeTopology {
	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;

public:
	explicit TreeTopology(openvdb::GridBase::ConstPtr grid);
	~TreeTopology() = default;

	void render(ViewerContext *context);
};

class VolumeBase : public Object {
protected:
	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;

	std::vector<glm::vec3> m_vertices;
	std::unique_ptr<TreeTopology> m_topology;

	openvdb::GridBase::Ptr m_grid;
	openvdb::Mat4R m_volume_matrix;  /* original volume matrix */

	float m_voxel_size;
	bool m_topology_changed, m_draw_topology;

	void updateGridTransform();
	void resampleGridVoxel();

	void setupData(openvdb::GridBase::Ptr grid);

public:
	VolumeBase();
	explicit VolumeBase(openvdb::GridBase::Ptr grid);
	~VolumeBase() = default;

	void update();

	float voxelSize() const;
	void setVoxelSize(const float voxel_size);

	TreeTopology *topology() const { return m_topology.get(); }
};
