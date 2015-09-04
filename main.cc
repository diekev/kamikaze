#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>

#include "viewer.h"

namespace  {

Viewer *viewer = nullptr;

void OnInit(const char *filename)
{
	if (viewer == nullptr) {
		viewer = new Viewer;
	}

	viewer->init(filename);
}

void OnShutDownCB()
{
	if (viewer) {
		viewer->shutDown();
	}

	delete viewer;
}

void OnRenderCB()
{
	if (viewer) {
		viewer->render();
	}
}

void OnResizeCB(int w, int h)
{
	if (viewer) {
		viewer->resize(w, h);
	}
}

void OnMouseDownCB(int button, int s, int x, int y)
{
	if (viewer) {
		viewer->mouseDownEvent(button, s, x, y);
	}
}

void OnMouseMoveCB(int x, int y)
{
	if (viewer) {
		viewer->mouseMoveEvent(x, y);
	}
}

}  /* namespace */

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);

	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("ikirin");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << '\n';
		return 1;
	}

	OnInit(argv[1]);

	glutCloseFunc(OnShutDownCB);
	glutDisplayFunc(OnRenderCB);
	glutReshapeFunc(OnResizeCB);
	glutMouseFunc(OnMouseDownCB);
	glutMotionFunc(OnMouseMoveCB);
	glutMainLoop();

	return 0;
}
