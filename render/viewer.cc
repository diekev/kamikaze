/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include <QApplication>
#include <QColorDialog>
#include <QKeyEvent>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "camera.h"
#include "scene.h"
#include "viewer.h"

#include "render/GPUBuffer.h"
#include "objects/grid.h"
#include "util/util_input.h"
#include "util/util_render.h"

Viewer::Viewer(QWidget *parent)
    : QGLWidget(parent)
    , m_mouse_button(MOUSE_NONE)
    , m_width(0)
    , m_height(0)
    , m_draw_grid(true)
    , m_bg(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
    , m_camera(new Camera())
    , m_grid(nullptr)
    , m_scene(nullptr)
{
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
	delete m_scene;
}

void Viewer::initializeGL()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
	}

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);

	m_grid = new Grid(20, 20);
	m_camera->updateViewDir();
}

void Viewer::resizeGL(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_camera->resize(w, h);
	m_width = w;
	m_height = h;
}

void Viewer::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_camera->updateViewDir();

	const auto &view_dir = m_camera->viewDir();
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	const auto &MVP = P * MV;

	if (m_draw_grid) {
		m_grid->render(MVP);
	}

	if (m_scene != nullptr) {
		m_scene->render(MV, P, view_dir);
	}
}

void Viewer::mousePressEvent(QMouseEvent *e)
{
	const int x = e->pos().x();
	const int y = e->pos().y();

	const auto &modifier = QApplication::keyboardModifiers();

	switch (modifier) {
		case Qt::NoModifier:
			m_modifier = MOD_KEY_NONE;
			break;
		case Qt::ShiftModifier:
			m_modifier = MOD_KEY_SHIFT;
			break;
		case Qt::ControlModifier:
			m_modifier = MOD_KEY_CTRL;
			break;
		case Qt::AltModifier:
			m_modifier = MOD_KEY_ALT;
			break;
	}

	if (e->buttons() == Qt::MidButton) {
		m_mouse_button = MOUSE_MIDDLE;
	}
	else if (e->buttons() == Qt::LeftButton) {
		m_mouse_button = MOUSE_LEFT;
		intersectScene(x, y);
	}
	else if (e->buttons() == Qt::RightButton) {
		m_mouse_button = MOUSE_RIGHT;
	}
	else {
		m_mouse_button = MOUSE_NONE;
	}

	m_camera->mouseDownEvent(x, y);

	update();
}

void Viewer::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouse_button == MOUSE_NONE) {
		return;
	}

	const int x = e->pos().x();
	const int y = e->pos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
	update();
}

void Viewer::mouseReleaseEvent(QMouseEvent *e)
{
	m_mouse_button = MOUSE_NONE;
	update();
}

void Viewer::keyPressEvent(QKeyEvent *e)
{
	m_scene->keyboardEvent(e->key());
	update();
}

void Viewer::wheelEvent(QWheelEvent *e)
{
	if (e->delta() < 0) {
		m_mouse_button = MOUSSE_SCROLL_DOWN;
	}
	else {
		m_mouse_button = MOUSSE_SCROLL_UP;
	}

	m_camera->mouseWheelEvent(m_mouse_button);
	update();
}

void Viewer::setScene(Scene *scene)
{
	m_scene = scene;
}

void Viewer::intersectScene(int x, int y)
{
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();

	glm::vec3 start = glm::unProject(glm::vec3(x, m_height - y, 0), MV, P,
	                                 glm::vec4(0, 0, m_width, m_height));
	glm::vec3 end = glm::unProject(glm::vec3(x, m_height - y, 1), MV, P,
	                               glm::vec4(0, 0, m_width, m_height));

	Ray ray;
	ray.pos = m_camera->pos();
	ray.dir = glm::normalize(end - start);

	m_scene->intersect(ray);
}

void Viewer::changeBackground()
{
	QColor color = QColorDialog::getColor();

	if (color.isValid()) {
		m_bg = glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.0f);
		glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
		update();
	}
}

void Viewer::drawGrid(bool b)
{
	m_draw_grid = b;
	update();
}
