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

typedef struct GPUVolumeShader GPUVolumeShader;
GPUVolumeShader volume_shader;

void OnInit()
{
	if (!init_volume_shader(volume_shader)) {
		std::cerr << "Initialisation of the volume data failed!\n";
		return;
	}

	glClearColor(bg.r, bg.g, bg.b, bg.a);

	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T, glm::radians(rX), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, glm::radians(rY), glm::vec3(0.0f, 1.0f, 0.0f));

	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	setup_volume_render(volume_shader, viewDir);

	std::cout << "Initialization successfull!\n";
}

void OnShutDown()
{
	delete_volume_shader(volume_shader);
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

	render_volume(volume_shader, viewDir, MVP, bViewRotated);

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

	OnInit();
	glutCloseFunc(OnShutDown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutMainLoop();

	return 0;
}
