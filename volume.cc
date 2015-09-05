#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <openvdb/openvdb.h>

#include "volume.h"

const float EPSILON = 0.0001f;

void convert_grid(const openvdb::FloatGrid &grid, GLfloat *data,
                  const openvdb::Coord &min, const openvdb::Coord &max)
{
	using namespace openvdb;

	FloatGrid::ConstAccessor acc = grid.getAccessor();
	math::Coord ijk;
	int &x = ijk[0], &y = ijk[1], &z = ijk[2];

	auto index = 0;
	for (z = min[2]; z < max[2]; ++z) {
		for (y = min[1]; y < max[1]; ++y) {
			for (x = min[0]; x < max[0]; ++x, ++index) {
				data[index] = acc.getValue(ijk);
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
    , m_min(glm::vec3(0.0f))
    , m_max(glm::vec3(0.0f))
    , m_size(glm::vec3(0.0f))
    , m_inv_size(glm::vec3(0.0f))
    , m_num_slices(256)
{}

VolumeShader::~VolumeShader()
{
	m_shader.deleteShaderProgram();

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteTextures(1, &m_texture_id);
}

bool VolumeShader::loadVolumeFile(const std::string &volume_file)
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
			return false;
		}

		CoordBBox bbox = grid->evalActiveVoxelBoundingBox();
		Coord bbox_min = bbox.min();
		Coord bbox_max = bbox.max();

		/* Get resolution */
		auto extent = bbox_max - bbox_min;
		const int X_DIM = extent[0];
		const int Y_DIM = extent[1];
		const int Z_DIM = extent[2];

		/* Compute grid size */
		Vec3f min = grid->transform().indexToWorld(bbox_min);
		Vec3f max = grid->transform().indexToWorld(bbox_max);

		for (int i(0); i < 3; ++i) {
			m_min[i] = min[i];
			m_max[i] = max[i];
		}

		m_size = (m_max - m_min);
		m_inv_size = 1.0f / m_size;

#if 0
		printf("Dimensions: %d, %d, %d\n", X_DIM, Y_DIM, Z_DIM);
		printf("Min: %f, %f, %f\n", min[0], min[1], min[2]);
		printf("Max: %f, %f, %f\n", max[0], max[1], max[2]);
#endif

		/* Copy data */
		GLfloat *data = new GLfloat[X_DIM * Y_DIM * Z_DIM];
		convert_grid(*grid, data, bbox_min, bbox_max);

		glGenTextures(1, &m_texture_id);
		glBindTexture(GL_TEXTURE_3D, m_texture_id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, X_DIM, Y_DIM, Z_DIM, 0, GL_RED, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_3D);

		file.close();
		delete [] data;
		return true;
	}

	std::cerr << "Unable to open file \'" << volume_file << "\'\n";
	return false;
}

#if 0
void VolumeShader::slice(const glm::vec3 &dir)
{
	const glm::vec3 vertices[8] = {
	    glm::vec3(m_min[0], m_min[1], m_min[2]),
	    glm::vec3(m_max[0], m_min[1], m_min[2]),
	    glm::vec3(m_max[0], m_max[1], m_min[2]),
	    glm::vec3(m_min[0], m_max[1], m_min[2]),
	    glm::vec3(m_min[0], m_min[1], m_max[2]),
	    glm::vec3(m_max[0], m_min[1], m_max[2]),
	    glm::vec3(m_max[0], m_max[1], m_max[2]),
	    glm::vec3(m_min[0], m_max[1], m_max[2])
	};

	const int edges[12][2] = {
	    { 0, 1 }, { 1, 2 }, { 2, 3 },
	    { 3, 0 }, { 0, 4 }, { 1, 5 },
	    { 2, 6 }, { 3, 7 }, { 4, 5 },
	    { 5, 6 }, { 6, 7 }, { 7, 4 }
	};

	const int edge_list[8][12] = {
	    { 0, 1, 5, 6, 4, 8, 11, 9, 3, 7, 2, 10 },
	    { 0, 4, 3, 11, 1, 2, 6, 7, 5, 9, 8, 10 },
	    { 1, 5, 0, 8, 2, 3, 7, 4, 6, 10, 9, 11 },
	    { 7, 11, 10, 8, 2, 6, 1, 9, 3, 0, 4, 5 },
	    { 8, 5, 9, 1, 11, 10, 7, 6, 4, 3, 0, 2 },
	    { 9, 6, 10, 2, 8, 11, 4, 7, 5, 0, 1, 3 },
	    { 9, 8, 5, 4, 6, 1, 2, 0, 10, 7, 11, 3 },
	    { 10, 9, 6, 5, 7, 2, 3, 1, 11, 4, 8, 0 }
	};

	/* get the max and min distance of eacg vertex of the unit cube
	 * in the viewing direction
	 */
	float max_dist = glm::dot(dir, vertices[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 1; i < 8; ++i) {
		/* get the distance between the current unit cube vertex and
		 * the view vector by dot product
		 */
		float dist = glm::dot(dir, vertices[i]);

		if (dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		if (dist < min_dist) {
			min_dist = dist;
		}
	}

	/* int max_dim = FindAbsMax(viewDir); */
	max_dist -= EPSILON;
	min_dist += EPSILON;

	/* start and direction vectors */
	glm::vec3 vecStart[12], vecDir[12];
	/* lambda intersection values */
	float lambda[12], lambda_inc[12];
	float denom = 0.0f;

	/* set the minimum distance as the plane_dist
	 * subtract the max and min distance and divide by the total number of
	 * slices to get the plane increment
	 */
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist - min_dist) / static_cast<float>(m_num_slices);

	/* for all egdes */
	for (int i = 0; i < 12; ++i) {
		/* get the start position vertex by table lookup */
		vecStart[i] = vertices[edges[edge_list[max_index][i]][0]];

		/* get the direction by table lookup */
		vecDir[i] = vertices[edges[edge_list[max_index][i]][1]] - vecStart[i];

		/* do a dot of vecDir and the view direction vector */
		denom = glm::dot(vecDir[i], dir);

		/* determine the plane intersection parameter (lambda) and
		 * plane intersection parameter increment (lambda_inc)
		 */
		if (1.0f + denom != 1.0f) {
			lambda_inc[i] = plane_dist_inc / denom;
			lambda[i] = (plane_dist - glm::dot(vecStart[i], dir)) / denom;
		}
		else {
			lambda[i] = -1.0f;
			lambda_inc[i] = 0.0f;
		}
	}

	/* for a plane and sub intersection, we can have a minimum of 3 and
	 * a maximum of 6 vertex polygon
	 */
	glm::vec3 intersections[6];
	float dL[12];

	for (int i = m_num_slices - 1; i >= 0; --i) {
		for (int e = 0; e < 12; ++e) {
			dL[e] = lambda[e] + i * lambda_inc[e];
		}

		/*if the values are between 0-1, we have an intersection at the current edge */
		/*repeat the same for all 12 edges */
		if ((dL[0] >= 0.0f) && (dL[0] < 1.0f)) {
			intersections[0] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0f) && (dL[1] < 1.0f)) {
			intersections[0] = vecStart[1] + dL[1] * vecDir[1];
		}
		else if ((dL[3] >= 0.0f) && (dL[3] < 1.0f)) {
			intersections[0] = vecStart[3] + dL[3] * vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0f) && (dL[2] < 1.0f)) {
			intersections[1] = vecStart[2] + dL[2] * vecDir[2];
		}
		else if ((dL[0] >= 0.0f) && (dL[0] < 1.0f)) {
			intersections[1] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0f) && (dL[1] < 1.0f)) {
			intersections[1] = vecStart[1] + dL[1] * vecDir[1];
		}
		else {
			intersections[1] = vecStart[3] + dL[3] * vecDir[3];
		}

		if ((dL[4] >= 0.0f) && (dL[4] < 1.0f)) {
			intersections[2] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0f) && (dL[5] < 1.0f)) {
			intersections[2] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersections[2] = vecStart[7] + dL[7] * vecDir[7];
		}

		if ((dL[6] >= 0.0f) && (dL[6] < 1.0f)) {
			intersections[3] = vecStart[6] + dL[6] * vecDir[6];
		}
		else if ((dL[4] >= 0.0f) && (dL[4] < 1.0f)) {
			intersections[3] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0f) && (dL[5] < 1.0f)) {
			intersections[3] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersections[3] = vecStart[7] + dL[7] * vecDir[7];
		}

		if ((dL[8] >= 0.0f) && (dL[8] < 1.0f)) {
			intersections[4] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0f) && (dL[9] < 1.0f)) {
			intersections[4] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersections[4] = vecStart[11] + dL[11] * vecDir[11];
		}

		if ((dL[10]>= 0.0f) && (dL[10]< 1.0f)) {
			intersections[5] = vecStart[10] + dL[10] * vecDir[10];
		}
		else if ((dL[8] >= 0.0f) && (dL[8] < 1.0f)) {
			intersections[5] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0f) && (dL[9] < 1.0f)) {
			intersections[5] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersections[5] = vecStart[11] + dL[11] * vecDir[11];
		}

		int indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5 };

		for (int i = 0; i < 12; ++i) {
			m_texture_slices[count++] = intersections[indices[i]] * m_inv_size;
		}
	}

	/* update buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_texture_slices), &(m_texture_slices[0].x));
}
#else

void VolumeShader::slice(const glm::vec3 &dir)
{
	auto view_dir = glm::normalize(dir);
	int axis = axis_dominant_v3_single(view_dir);
	sliceAxisAligned(view_dir, axis);
}

void VolumeShader::sliceAxisAligned(const glm::vec3 &view_dir, const int axis)
{
	auto count = 0;
	auto depth = m_min[axis];
	auto slice_size = m_size[axis] / m_num_slices;

	/* always process slices in back to front order! */
	if (view_dir[axis] > 0.0f) {
		depth = m_max[axis];
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

		m_texture_slices[count++] = vertices[axis][0] * m_inv_size;
		m_texture_slices[count++] = vertices[axis][1] * m_inv_size;
		m_texture_slices[count++] = vertices[axis][2] * m_inv_size;
		m_texture_slices[count++] = vertices[axis][0] * m_inv_size;
		m_texture_slices[count++] = vertices[axis][2] * m_inv_size;
		m_texture_slices[count++] = vertices[axis][3] * m_inv_size;

		depth += slice_size;
	}

	/* update buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_texture_slices), &(m_texture_slices[0].x));
}
#endif

bool VolumeShader::init(const std::string &filename)
{
	if (loadVolumeFile(filename)) {
		m_shader.loadFromFile(GL_VERTEX_SHADER, "shader/texture_slicer.vert");
		m_shader.loadFromFile(GL_FRAGMENT_SHADER, "shader/texture_slicer.frag");

		m_shader.createAndLinkProgram();

		m_shader.use();

		m_shader.addAttribute("vVertex");
		m_shader.addUniform("MVP");
		m_shader.addUniform("offset");
		m_shader.addUniform("volume");
		m_shader.addUniform("lut");

		glUniform1i(m_shader("volume"), 0);
		glUniform1i(m_shader("lut"), 1);

		auto min = m_min * m_inv_size;
		glUniform3fv(m_shader("offset"), 1, &min[0]);

		m_shader.unUse();

		return true;
	}

	return false;
}

void VolumeShader::setupRender()
{
	/* setup the vertex array and buffer objects */
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	/* pass the sliced volume vectore to buffer output memory */
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_texture_slices), 0, GL_DYNAMIC_DRAW);

	/* enable vertex attribute array for position */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void VolumeShader::render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated)
{
	if (is_rotated) {
		slice(dir);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(m_vao);
	m_shader.use();
	glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glDrawArrays(GL_TRIANGLES, 0, MAX_SLICES * 12);
	m_shader.unUse();
	glDisable(GL_BLEND);
}

void VolumeShader::changeNumSlicesBy(int x)
{
	m_num_slices += x;
	m_num_slices = std::min(MAX_SLICES, std::max(m_num_slices, 3));
}
