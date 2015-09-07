#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>

#include <iostream>

#include "GLSLShader.h"

#include "camera.h"
#include "grid.h"
#include "viewer.h"
#include "volume.h"

Viewer::Viewer()
    : m_mouse_state(0)
    , m_bg(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f))
    , m_camera(new Camera())
    , m_grid(new Grid(20, 20))
    , m_volume_shader(new VolumeShader())
{}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
	delete m_volume_shader;
}

void Viewer::init(const char *filename, std::ostream &os)
{
	if (!m_volume_shader->init(filename, os)) {
		os << "Initialisation of the volume data failed!\n";
		return;
	}

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);

	m_camera->updateViewDir();
	m_volume_shader->slice(m_camera->viewDir());
}

void Viewer::shutDown()
{}

void Viewer::resize(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_camera->resize(w, h);
}

void Viewer::mouseDownEvent(int button, int s, int x, int y)
{
	if (button == GLUT_MIDDLE_BUTTON) {
		m_mouse_state = 0;
	}
	else {
		m_mouse_state = 1;
	}

	m_camera->mouseDownEvent(s, x, y);
}

void Viewer::mouseMoveEvent(int x, int y)
{
	m_camera->mouseMoveEvent(m_mouse_state, x, y);
	glutPostRedisplay();
}

void Viewer::keyboardEvent(unsigned char key, int /*x*/, int /*y*/)
{
	bool need_slicing = false;

	switch (key) {
		case '-':
			m_volume_shader->changeNumSlicesBy(-1);
			need_slicing = true;
			break;
		case '+':
			m_volume_shader->changeNumSlicesBy(1);
			need_slicing = true;
			break;
		case 'l':
			m_volume_shader->toggleUseLUT();
			break;
		case 'b':
			m_volume_shader->toggleBBoxDrawing();
			break;
	}

	if (need_slicing) {
		m_volume_shader->slice(m_camera->viewDir());
	}

	glutPostRedisplay();
}

void Viewer::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_camera->updateViewDir();
	auto view_dir = m_camera->viewDir();
	auto MVP = m_camera->MVP();

	m_grid->render(MVP);
	m_volume_shader->render(view_dir, MVP, m_camera->hasRotated());

	glutSwapBuffers();
}
