#include <fstream>

#include "volumesplatter.h"

VolumeSplatter::VolumeSplatter()
{
	m_res = glm::ivec3(256);
	m_total_cells = m_res.x * m_res.y * m_res.z;
}

VolumeSplatter::~VolumeSplatter()
{
	if (m_volume != nullptr) {
		delete [] m_volume;
		m_volume = nullptr;
	}
}

GLubyte VolumeSplatter::sampleVolume(const int x, const int y, const int z)
{
	auto index = (x + (y * m_res_x) + z * (m_res_x * m_res_y));

	if (index < 0) {
		index = 0;
	}

	if (index >= m_total_cells) {
		index = m_total_cells - 1;
	}

	return m_volume[index];
}

glm::vec3 VolumeSplatter::getNormal(const int x, const int y, const int z)
{
	glm::vec3 N;

	N.x = (sampleVolume(int(x - m_scale.x), y, z) - sampleVolume(int(x + m_scale.x), y, z)) / (2.0f * m_scale.x);
	N.y = (sampleVolume(y, int(y - m_scale.y), z) - sampleVolume(y, int(y + m_scale.y), z)) / (2.0f * m_scale.y);
	N.z = (sampleVolume(x, y, int(z - m_scale.z)) - sampleVolume(x, y, int(z + m_scale.z))) / (2.0f * m_scale.z);

	return N;
}

void VolumeSplatter::sampleVoxel(const int x, const int y, const int z)
{
	GLubyte data = sampleVolume(x, y, z);

	if (data > m_iso_value) {
		Vertex v;
		v.pos.x = static_cast<float>(x);
		v.pos.y = static_cast<float>(y);
		v.pos.z = static_cast<float>(z);
		v.nor = getNormal(x, y, z);
		v.pos *= m_inv_res;
		m_vertices.push_back(v);
	}
}

void VolumeSplatter::setVolumeDimensions(const int xdim, const int ydim, const int zdim)
{
	m_res = glm::vec3(xdim, ydim, zdim);
	m_inv_res.x = 1.0f / m_res.x;
	m_inv_res.y = 1.0f / m_res.y;
	m_inv_res.z = 1.0f / m_res.z;
}

void VolumeSplatter::setNumSamplingVoxels(const int x, const int y, const int z)
{
	m_sampling_dist_x = x;
	m_sampling_dist_y = y;
	m_sampling_dist_z = z;
}

void VolumeSplatter::setIsoValue(const GLubyte value)
{
	m_iso_value = value;
}

bool VolumeSplatter::loadVolume(const std::string &filename)
{
	std::ifstream infile(filename.c_str(), std::ios_base::binary);

	if (infile.good()) {
		m_volume = new GLubyte[m_total_cells];
		infile.read(reinterpret_cast<char *>(m_volume), m_total_cells);

		return true;
	}

	return false;
}

void VolumeSplatter::SplatVolume()
{
	m_vertices.clear();

	auto dx = m_res.x / m_sampling_dist_x;
	auto dy = m_res.y / m_sampling_dist_y;
	auto dz = m_res.z / m_sampling_dist_z;

	for (int z = 0; z < m_res_z; z += dz) {
		for (int y = 0; y < m_res_y; z += dy) {
			for (int x = 0; x < m_res_x; x += dx) {
				sampleVoxel(x, y, z);
			}
		}
	}
}

size_t VolumeSplatter::numVertices()
{
	return m_vertices.size();
}

Vertex *VolumeSplatter::vertices()
{
	return &m_vertices[0];
}
