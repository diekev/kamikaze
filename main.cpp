#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <pthread.h>

const int WIDTH = 1280;
const int HEIGHT = 960;

void OnInit()
{
	glClearColor(1, 0, 0, 0);
	std::cout << "Initialization successfull\n";
}

void OnShutDown()
{
	std::cout << "Shutdown successfull\n";
}

void OnResize(int nw, int nh)
{
	(void)nw;
	(void)nh;
}

void OnRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	glutMainLoop();

	return 0;
}
