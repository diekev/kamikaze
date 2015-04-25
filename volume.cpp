#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.h"

// Resolution of the volume texture
#define XDIM 256
#define YDIM 256
#define ZDIM 256

std::string volume_file = "/home/kevin/Téléchargements/Engine256.raw";

GLuint textureID;
GLuint volumeVAO, volumeVBO;

const float EPSILON = 0.0001f;
const int MAX_SLICES = 512;

// modelview and projection
glm::mat4 MV, P;

glm::vec3 vTextureSlices[MAX_SLICES * 12];

GLSLShader shader;

// total number of slices curently used
int num_slices = 256;

//unit cube vertices
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

//unit cube edges
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};

const int edges[12][2]= {
	{0,1},{1,2},{2,3},
	{3,0},{0,4},{1,5},
	{2,6},{3,7},{4,5},
	{5,6},{6,7},{7,4}
};

glm::vec3 viewDir;

bool loadVolumeFile(const std::string &volume_file)
{
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if (infile.good()) {
		GLubyte *pData = new GLubyte[XDIM * YDIM * ZDIM];
		infile.read(reinterpret_cast<char *>(pData), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
		glGenerateMipmap(GL_TEXTURE_3D);

		return true;
	}

	return false;
}

//function to get the max (abs) dimension of the given vertex v
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

void sliceVolume()
{
	/* get the max and min distance of eacg vertex of the unit cube
	 * in the viewing direction
	 */
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 0; i < 8; ++i) {
		/* get the distance between the current unit cube vertex and
		 * the view vector by dot product
		 */
		float dist = glm::dot(viewDir, vertexList[i]);

		if (dist > max_dist) {
			max_dist = dist;
			max_index = 0;
		}

		if (dist < min_dist) {
			min_dist = dist;
		}
	}

	// int max_dim = FindAbsMax(viewDir);
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
	float plane_dist_inc = (max_dist - min_dist) / float(num_slices);

	/* for all egdes */
	for (int i = 0; i < 12; ++i) {
		/* get the start position vertex by table lookup */
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		/* get the direction by table lookup */
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

		/* do a dot of vecDir and the view direction vector */
		denom = glm::dot(vecDir[i], viewDir);

		/* determine the plane intersection parameter (lambda) and
		 * plane intersection parameter increment (lambda_inc)
		 */
		if (1.0f + denom != 1.0f) {
			lambda_inc[i] = plane_dist_inc / denom;
			lambda[i] = (plane_dist - glm::dot(vecStart[i], viewDir)) / denom;
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

		//if the values are between 0-1, we have an intersection at the current edge
		//repeat the same for all 12 edges
		if ((dL[0] >= 0.0f) && (dL[0] < 1.0f))	{
			intersections[0] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0f) && (dL[1] < 1.0f))	{
			intersections[0] = vecStart[1] + dL[1]*vecDir[1];
		}
		else if ((dL[3] >= 0.0f) && (dL[3] < 1.0f))	{
			intersections[0] = vecStart[3] + dL[3]*vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0f) && (dL[2] < 1.0f)) {
			intersections[1] = vecStart[2] + dL[2]*vecDir[2];
		}
		else if ((dL[0] >= 0.0f) && (dL[0] < 1.0f)) {
			intersections[1] = vecStart[0] + dL[0]*vecDir[0];
		}
		else if ((dL[1] >= 0.0f) && (dL[1] < 1.0f)) {
			intersections[1] = vecStart[1] + dL[1]*vecDir[1];
		}
		else {
			intersections[1] = vecStart[3] + dL[3]*vecDir[3];
		}

		if ((dL[4] >= 0.0f) && (dL[4] < 1.0f)) {
			intersections[2] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0f) && (dL[5] < 1.0f)) {
			intersections[2] = vecStart[5] + dL[5]*vecDir[5];
		}
		else {
			intersections[2] = vecStart[7] + dL[7]*vecDir[7];
		}

		if ((dL[6] >= 0.0f) && (dL[6] < 1.0f)) {
			intersections[3] = vecStart[6] + dL[6]*vecDir[6];
		}
		else if ((dL[4] >= 0.0f) && (dL[4] < 1.0f)) {
			intersections[3] = vecStart[4] + dL[4]*vecDir[4];
		}
		else if ((dL[5] >= 0.0f) && (dL[5] < 1.0f)) {
			intersections[3] = vecStart[5] + dL[5]*vecDir[5];
		}
		else {
			intersections[3] = vecStart[7] + dL[7]*vecDir[7];
		}

		if ((dL[8] >= 0.0f) && (dL[8] < 1.0f)) {
			intersections[4] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0f) && (dL[9] < 1.0f)) {
			intersections[4] = vecStart[9] + dL[9]*vecDir[9];
		}
		else {
			intersections[4] = vecStart[11] + dL[11]*vecDir[11];
		}

		if ((dL[10]>= 0.0f) && (dL[10]< 1.0f)) {
			intersections[5] = vecStart[10] + dL[10]*vecDir[10];
		}
		else if ((dL[8] >= 0.0f) && (dL[8] < 1.0f)) {
			intersections[5] = vecStart[8] + dL[8]*vecDir[8];
		}
		else if ((dL[9] >= 0.0f) && (dL[9] < 1.0f)) {
			intersections[5] = vecStart[9] + dL[9]*vecDir[9];
		}
		else {
			intersections[5] = vecStart[11] + dL[11]*vecDir[11];
		}

		int indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5};

		for (int i = 0; i < 12; ++i) {
			vTextureSlices[count++] = intersections[indices[i]];
		}
	}

	/* update buffer object */
	glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vTextureSlices), &(vTextureSlices[0].x));
}

#include <iostream>

// camera transform variables
int state = 0, oldX = 0, oldY = 0;
float rX = 4.0f, rY = 50.0f, dist = -2.0f;

glm::vec4 bg = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

bool bViewRotated = false;

void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN) {
		oldX = x;
		oldY = y;
	}

	if (button == GLUT_MIDDLE_BUTTON) {
		state = 0;
	}
	else {
		state = 1;
	}

	if (s == GLUT_UP) {
		bViewRotated = false;
	}
}

void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist += (y - oldY) / 50.0f;
	}
	else {
		rX += (y - oldY) / 5.0f;
		rY += (x - oldX) / 5.0f;
		bViewRotated = true;
	}

	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

void OnInit()
{
	shader.LoadFromFile(GL_VERTEX_SHADER, "shader/texture_slicer.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shader/texture_slicer.frag");

	shader.CreateAndLinkProgram();
	shader.Use();
	shader.AddAttribute("vVertex");
	shader.AddUniform("MVP");
	shader.AddUniform("volume");
	glUniform1i(shader("volume"), 0);
	shader.UnUse();

	if (loadVolumeFile(volume_file)) {
		std::cout << "Volume data loaded succesfully.\n";
	}
	else {
		std::cerr << "Cannot load volume data!\n";
		return;
	}

	glClearColor(bg.r, bg.g, bg.b, bg.a);

	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T, rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	/* setup the vertex array and buffer objects */
	glGenVertexArrays(1, &volumeVAO);
	glGenBuffers(1, &volumeVBO);

	glBindVertexArray(volumeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, volumeVBO);

	// pass the sliced volume vectore to buffer output memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vTextureSlices), 0, GL_DYNAMIC_DRAW);

	// enable vertex attribute array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	sliceVolume();

	std::cout << "Initialisation succesfull!\n";
}

void OnShutDown()
{
	shader.DeleteShaderProgram();

	glDeleteVertexArrays(1, &volumeVAO);
	glDeleteBuffers(1, &volumeVBO);
	glDeleteTextures(1, &textureID);

	std::cout << "Shutdown succesfull!\n";
}

void OnResize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	P = glm::perspective(60.0f, (float)w/h, 0.1f, 1000.0f);
}

void OnRender()
{
	glm::mat4 Tr = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(Tr, rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MVP = P * MV;

	if (bViewRotated) {
		sliceVolume();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(volumeVAO);
	shader.Use();
	glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glDrawArrays(GL_TRIANGLES, 0, sizeof(vTextureSlices) / sizeof(vTextureSlices[0]));
	shader.UnUse();
	glDisable(GL_BLEND);
	glutSwapBuffers();
}

const int WIDTH = 1280;
const int HEIGHT = 960;

int main(int argc, char *argv[])
{
	int i = pthread_getconcurrency();
	(void)i;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Getting started with OpenGL 3.3");
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	else {
		if (GLEW_VERSION_3_3) {
			std::cout << "Driver supports OpenGL 3.3\nDetails:\n";
		}
	}

	std::cout << "\tUsing glew " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "\tVendor " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "\tRenderer " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "\tVersion " << glGetString(GL_VERSION) << std::endl;
	std::cout << "\tGLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	OnInit();
	glutCloseFunc(OnShutDown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);

	glutMainLoop();

	return 0;
}
