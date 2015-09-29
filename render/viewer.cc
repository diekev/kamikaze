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
#include <QKeyEvent>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>

#include "camera.h"
#include "scene.h"
#include "viewer.h"

#include "objects/grid.h"
#include "util/util_input.h"

Viewer::Viewer(QWidget *parent)
    : QGLWidget(parent)
    , m_mouse_button(0)
    , m_bg(glm::vec4(0.5f, 0.5f, 1.0f, 1.0f))
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
}

void Viewer::paintGL()
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
		m_mouse_button = MOUSSE_MIDDLE;
	}
	else if (e->buttons() == Qt::LeftButton) {
		m_mouse_button = MOUSSE_LEFT;
	}
	else if (e->buttons() == Qt::RightButton) {
		m_mouse_button = MOUSSE_RIGHT;
	}
	else {
		m_mouse_button = -1;
	}

	m_camera->mouseDownEvent(m_mouse_button, 0, x, y);

	update();
}

void Viewer::mouseMoveEvent(QMouseEvent *e)
{
	const int x = e->pos().x();
	const int y = e->pos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
	update();
}

void Viewer::mouseReleaseEvent(QMouseEvent *e)
{
	m_mouse_button = -1;
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

	m_camera->mouseDownEvent(m_mouse_button, -1, 0, 0);
	update();
}

void Viewer::setScene(Scene *scene)
{
	m_scene = scene;
}
