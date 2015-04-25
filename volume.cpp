#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "volume.h"

/* Resolution of the volume texture */
#define XDIM 256
#define YDIM 256
#define ZDIM 256

std::string volume_file = "/home/kevin/Téléchargements/Engine256.raw";

const float EPSILON = 0.0001f;

/* total number of slices curently used */
int num_slices = 256;

/* unit cube vertices */
glm::vec3 vertexList[8] = {
	glm::vec3(-0.5,-0.5,-0.5),
	glm::vec3( 0.5,-0.5,-0.5),
	glm::vec3(0.5, 0.5,-0.5),
	glm::vec3(-0.5, 0.5,-0.5),
	glm::vec3(-0.5,-0.5, 0.5),
	glm::vec3(0.5,-0.5, 0.5),
	glm::vec3( 0.5, 0.5, 0.5),
	glm::vec3(-0.5, 0.5, 0.5)
};

/* unit cube edges */
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, /* v0 is front */
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, /* v1 is front */
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, /* v2 is front */
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, /* v3 is front */
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, /* v4 is front */
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, /* v5 is front */
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, /* v6 is front */
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  /* v7 is front */
};

const int edges[12][2]= {
	{0,1},{1,2},{2,3},
	{3,0},{0,4},{1,5},
	{2,6},{3,7},{4,5},
	{5,6},{6,7},{7,4}
};

VolumeShader::VolumeShader()
{}

VolumeShader::~VolumeShader()
{
	m_shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteTextures(1, &m_texture_id);
}

bool VolumeShader::loadVolumeFile(const std::string &volume_file)
{
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if (infile.good()) {
		GLubyte *data = new GLubyte[XDIM * YDIM * ZDIM];
		infile.read(reinterpret_cast<char *>(data), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		glGenTextures(1, &m_texture_id);
		glBindTexture(GL_TEXTURE_3D, m_texture_id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_3D);

		delete [] data;

		return true;
	}

	std::cerr << "Unable to open file \'" << volume_file << "\'\n";

	return false;
}

#if 0
/* function to get the max (abs) dimension of the given vertex v */
int FindAbsMax(glm::vec3 v) {
	v = glm::abs(v);
	int max_dim = 0;
	float val = v.x;
	if(v.y>val) {
		val = v.y;
		max_dim = 1;
	}
	if(v.z > val) {
		val = v.z;
		max_dim = 2;
	}
	return max_dim;
}
#endif

void VolumeShader::slice(const glm::vec3 &dir)
{
	/* get the max and min distance of eacg vertex of the unit cube
	 * in the viewing direction
	 */
	float max_dist = glm::dot(dir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 1; i < 8; ++i) {
		/* get the distance between the current unit cube vertex and
		 * the view vector by dot product
		 */
		float dist = glm::dot(dir, vertexList[i]);

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
	float plane_dist_inc = (max_dist - min_dist) / static_cast<float>(num_slices);

	/* for all egdes */
	for (int i = 0; i < 12; ++i) {
		/* get the start position vertex by table lookup */
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		/* get the direction by table lookup */
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

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

	for (int i = num_slices - 1; i >= 0; --i) {
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
			m_texture_slices[count++] = intersections[indices[i]];
		}
	}

	/* update buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_texture_slices), &(m_texture_slices[0].x));
}

bool VolumeShader::init()
{
	GLSLShader *shader = &m_shader;

	shader->LoadFromFile(GL_VERTEX_SHADER, "shader/texture_slicer.vert");
	shader->LoadFromFile(GL_FRAGMENT_SHADER, "shader/texture_slicer.frag");

	shader->CreateAndLinkProgram();
	shader->Use();
	shader->AddAttribute("vVertex");
	shader->AddUniform("MVP");
	shader->AddUniform("volume");
	shader->AddUniform("lut");
	glUniform1i((*shader)("volume"), 0);
	glUniform1i((*shader)("lut"), 1);
	shader->UnUse();

	return loadVolumeFile(volume_file);
}

void VolumeShader::setupRender(const glm::vec3 &dir)
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

	slice(dir);
}

void VolumeShader::render(const glm::vec3 &dir, const glm::mat4 &MVP, const bool is_rotated)
{
	if (is_rotated) {
		slice(dir);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(m_vao);
	m_shader.Use();
	glUniformMatrix4fv(m_shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glDrawArrays(GL_TRIANGLES, 0, sizeof(m_texture_slices) / sizeof(m_texture_slices[0]));
	m_shader.UnUse();
	glDisable(GL_BLEND);
}

