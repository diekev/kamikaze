#ifndef __VOLUMESPLATTER_H__
#define __VOLUMESPLATTER_H__

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
	glm::vec3 pos, nor;
};

class VolumeSplatter {
protected:
	GLubyte sampleVolume(const int x, const int y, const int z);
	glm::vec3 getNormal(const int x, const int y, const int z);
	void sampleVoxel(const int x, const int y, const int z);

	int m_res_x, m_res_y, m_res_z, m_total_cells;
	glm::ivec3 m_res;
	glm::vec3 m_inv_res;

	int m_sampling_dist_x, m_sampling_dist_y, m_sampling_dist_z;

	GLubyte *m_volume;
	GLubyte m_iso_value;
	glm::vec3 m_scale;
	std::vector<Vertex> m_vertices;

public:
	VolumeSplatter();
	~VolumeSplatter();

	void setVolumeDimensions(const int xdim, const int ydim, const int zdim);
	void setNumSamplingVoxels(const int x, const int y, const int z);
	void setIsoValue(const GLubyte value);
	bool loadVolume(const std::string &filename);
	void SplatVolume();
	size_t numVertices();
	Vertex *vertices();
};

#endif // __VOLUMESPLATTER_H__
