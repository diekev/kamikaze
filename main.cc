#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#include <iostream>

#include "viewer.h"

namespace {

Viewer *viewer = nullptr;

bool initViewer(const char *filename, std::ostream &os)
{
	if (viewer == nullptr) {
		viewer = new Viewer;
	}

	return viewer->init(filename, os);
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

void OnKeyEventCB(unsigned char key, int x, int y)
{
	if (viewer) {
		viewer->keyboardEvent(key, x, y);
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

	std::ostream &os = std::cerr;

	if (GLEW_OK != err) {
		os << "Error: " << glewGetErrorString(err) << '\n';
		return 1;
	}

	if (!initViewer(argv[1], os)) {
		OnShutDownCB();
		return 1;
	}

	glutCloseFunc(OnShutDownCB);
	glutDisplayFunc(OnRenderCB);
	glutReshapeFunc(OnResizeCB);
	glutMouseFunc(OnMouseDownCB);
	glutMotionFunc(OnMouseMoveCB);
	glutKeyboardFunc(OnKeyEventCB);
	glutMainLoop();

	return 0;
}
