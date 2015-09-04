#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <pthread.h>

#include "volume.h"

const int WIDTH = 1280;
const int HEIGHT = 960;

/* camera transform variables */
int state = 0, oldX = 0, oldY = 0;
float rX = 4.0f, rY = 50.0f, dist = -2.0f;

/* modelview and projection */
glm::mat4 MV, P;

glm::vec3 viewDir;

glm::vec4 bg = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);

bool bViewRotated = false;

GLuint tfTexID;

/* transfer function (lookup table) colour values */
const glm::vec4 jet_values[9] = {
	glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
	glm::vec4(0.0f, 0.0f, 1.0f, 0.1f),
	glm::vec4(0.0f, 0.5f, 1.0f, 0.3f),
	glm::vec4(0.0f, 1.0f, 1.0f, 0.3f),
	glm::vec4(0.5f, 1.0f, 0.5f, 0.75f),
	glm::vec4(1.0f, 1.0f, 0.0f, 0.8f),
	glm::vec4(1.0f, 0.5f, 0.0f, 0.6f),
	glm::vec4(1.0f, 0.0f, 0.0f, 0.5f),
	glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
};

/* Function to generate interpolated colours from the set of colours values
 * (jet_values). This function first calculates the amount of increments for
 * each component and the index difference. Then it linearly interpolates the
 * adjacent values to get the interpolated result.
 */
void loadTransferFunction()
{
	float pData[256][4];
	int indices[9];

	/* fill the colour values at the place where the colour shuld be after
	 * interpolation */
	for (int i = 0; i < 9; ++i) {
		auto index = i * 28;
		pData[index][0] = jet_values[i].x;
		pData[index][1] = jet_values[i].y;
		pData[index][2] = jet_values[i].z;
		pData[index][3] = jet_values[i].w;
		indices[i] = index;
	}

	/* for each adjacent pair of colours, find the difference in the rgba values
	 * and then interpolate */
	for (int j = 0; j < 9 - 1; ++j) {
		auto dDataR = (pData[indices[j + 1]][0] - pData[indices[j]][0]);
		auto dDataG = (pData[indices[j + 1]][1] - pData[indices[j]][1]);
		auto dDataB = (pData[indices[j + 1]][2] - pData[indices[j]][2]);
		auto dDataA = (pData[indices[j + 1]][3] - pData[indices[j]][3]);

		auto dIndex = indices[j + 1] - indices[j];

		auto dDatatIncR = dDataR / static_cast<float>(dIndex);
		auto dDatatIncG = dDataG / static_cast<float>(dIndex);
		auto dDatatIncB = dDataB / static_cast<float>(dIndex);
		auto dDatatIncA = dDataA / static_cast<float>(dIndex);

		for (int i = indices[j] + 1; i < indices[j + 1]; ++i) {
			pData[i][0] = (pData[i - 1][0] + dDatatIncR);
			pData[i][1] = (pData[i - 1][1] + dDatatIncG);
			pData[i][2] = (pData[i - 1][2] + dDatatIncB);
			pData[i][3] = (pData[i - 1][3] + dDatatIncA);
		}
	}

	glGenTextures(1, &tfTexID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, tfTexID);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, pData);
}

VolumeShader volumeShader;

void OnInit(const char *filename)
{
	if (!volumeShader.init(filename)) {
		std::cerr << "Initialisation of the volume data failed!\n";
		return;
	}

	/* load the transfuer function data and generate the transfer look up table */
	loadTransferFunction();

	glClearColor(bg.r, bg.g, bg.b, bg.a);

	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T, glm::radians(rX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, glm::radians(rY), glm::vec3(0.0f, 1.0f, 0.0f));

	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	volumeShader.setupRender();
	volumeShader.slice(viewDir);

	std::cout << "Initialization successfull!\n";
}

void OnShutDown()
{
	glDeleteTextures(1, &tfTexID);
	std::cout << "Shutdown successfull!\n";
}

void OnResize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	P = glm::perspective(glm::radians(60.0f), (float)w/h, 0.1f, 1000.0f);
}

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

void OnRender()
{
	glm::mat4 Tr = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(Tr, glm::radians(rX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, glm::radians(rY), glm::vec3(0.0f, 1.0f, 0.0f));

	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 MVP = P * MV;

	volumeShader.render(viewDir, MVP, bViewRotated);

	glutSwapBuffers();
}

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

	OnInit(argv[1]);

	glutCloseFunc(OnShutDown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMainLoop();

	return 0;
}
