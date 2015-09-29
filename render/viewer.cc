#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#include <openvdb/openvdb.h>

#include <iostream>

#include "objects/grid.h"

#include "camera.h"
#include "scene.h"
#include "viewer.h"

#include "util/util_input.h"

Viewer::Viewer()
    : m_mouse_button(0)
    , m_bg(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f))
    , m_camera(new Camera())
    , m_grid(new Grid(20, 20))
    , m_scene(nullptr)
{}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
	delete m_scene;
}

void Viewer::init()
{
	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
	m_camera->updateViewDir();
}

void Viewer::resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_camera->resize(w, h);
}

void Viewer::mouseDownEvent(int button, int s, int x, int y)
{
	bool need_redisplay = false;

	m_modifier = glutGetModifiers();

	if (button == GLUT_MIDDLE_BUTTON) {
		m_mouse_button = MOUSSE_MIDDLE;
	}
	else if ((button == 3 || button == 4) && (s != GLUT_UP)) {
		m_mouse_button = button;
		need_redisplay = true;
	}
	else {
		m_mouse_button = -1;
	}

	m_camera->mouseDownEvent(m_mouse_button, s, x, y);

	if (need_redisplay) {
		glutPostRedisplay();
	}
}

void Viewer::mouseMoveEvent(int x, int y)
{
	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
	glutPostRedisplay();
}

void Viewer::keyboardEvent(unsigned char key, int /*x*/, int /*y*/)
{
	m_scene->keyboardEvent(key);
	glutPostRedisplay();
}

void Viewer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_camera->updateViewDir();

	const auto &view_dir = m_camera->viewDir();
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	const auto &MVP = P * MV;

	m_grid->render(MVP);

	if (m_scene != nullptr) {
		m_scene->render(view_dir, MV, P);
	}

	glutSwapBuffers();
}

void Viewer::setScene(Scene *scene)
{
	m_scene = scene;
}
