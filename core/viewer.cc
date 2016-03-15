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

#include "viewer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

#include <kamikaze/context.h>

#include <QApplication>
#include <QColorDialog>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "camera.h"
#include "scene.h"

#include "grid.h"
#include "util/util_input.h"

Viewer::Viewer(QWidget *parent)
    : QGLWidget(parent)
    , m_mouse_button(MOUSE_NONE)
    , m_width(0)
    , m_height(0)
    , m_draw_grid(true)
    , m_bg(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
    , m_camera(new Camera(m_width, m_height))
    , m_grid(nullptr)
    , m_scene(nullptr)
    , m_timer(new QTimer(this))
    , m_context(new ViewerContext)
{
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
//	delete m_scene;
	delete m_context;
}

void Viewer::initializeGL()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
	}

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	m_grid = new Grid(20, 20);
	m_camera->update();

	m_timer->start(1000 / 24);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* setup stencil mask for outlining active object */
	glStencilFunc(GL_ALWAYS, 1, 0xff);
	glStencilMask(0xff);

	m_camera->update();

	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	const auto &MVP = P * MV;

	m_context->setView(m_camera->dir());
	m_context->setModelview(MV);
	m_context->setProjection(P);
	m_context->setMVP(MVP);
	m_context->setNormal(glm::inverseTranspose(glm::mat3(MV)));

	if (m_draw_grid) {
		m_grid->render(MVP);
	}

	if (m_scene != nullptr) {
		m_scene->render(m_context);
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
		selectObject(x, y);
	}
	else if (e->buttons() == Qt::RightButton) {
		m_mouse_button = MOUSE_RIGHT;
	}
	else {
		m_mouse_button = MOUSE_NONE;
	}

	m_camera->mouseDownEvent(x, y);
}

void Viewer::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouse_button == MOUSE_NONE) {
		return;
	}

	const int x = e->pos().x();
	const int y = e->pos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
}

void Viewer::mouseReleaseEvent(QMouseEvent */*e*/)
{
	m_mouse_button = MOUSE_NONE;
	update();
}

void Viewer::keyPressEvent(QKeyEvent *e)
{
	m_scene->keyboardEvent(e->key());
}

void Viewer::wheelEvent(QWheelEvent *e)
{
	if (e->delta() < 0) {
		m_mouse_button = MOUSE_SCROLL_DOWN;
	}
	else {
		m_mouse_button = MOUSE_SCROLL_UP;
	}

	m_camera->mouseWheelEvent(m_mouse_button);
}

void Viewer::setScene(Scene *scene)
{
	m_scene = scene;
}

void Viewer::intersectScene(int x, int y) const
{
	const glm::vec3 start = unproject(glm::vec3(x, m_height - y, 0.0f));
	const glm::vec3 end = unproject(glm::vec3(x, m_height - y, 1.0f));

	Ray ray;
	ray.pos = m_camera->pos();
	ray.dir = glm::normalize(end - start);

	m_scene->intersect(ray);
}

void Viewer::selectObject(int x, int y) const
{
	float z;
	glReadPixels(x, m_height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

	const glm::vec3 pos = unproject(glm::vec3(x, m_height - y, z));
	m_scene->selectObject(pos);
}

glm::vec3 Viewer::unproject(const glm::vec3 &pos) const
{
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	return glm::unProject(pos, MV, P, glm::vec4(0, 0, m_width, m_height));
}

void Viewer::changeBackground()
{
	QColor color = QColorDialog::getColor();

	if (color.isValid()) {
		m_bg = glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.0f);
		glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
	}
}

void Viewer::drawGrid(bool b)
{
	m_draw_grid = b;
}

GraphicsViewer::GraphicsViewer(QWidget *parent)
    : QGraphicsView(parent)
{
//	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
//	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

//	m_glscene = new OpenGLScene;

//	setScene(m_glscene);
}

GraphicsViewer::~GraphicsViewer()
{
	delete m_glscene;
}

OpenGLScene *GraphicsViewer::GLScene() const
{
	return m_glscene;
}

void GraphicsViewer::GLScene(OpenGLScene *scene)
{
	m_glscene = scene;
	setScene(m_glscene);
}

void GraphicsViewer::resizeEvent(QResizeEvent *event)
{
	if (scene()) {
		scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
		QGraphicsView::resizeEvent(event);
	}
}

OpenGLScene::OpenGLScene()
    : QGraphicsScene()
    , m_width(0)
    , m_height(0)
    , m_camera(new Camera(m_width, m_height))
    , m_grid(nullptr)
    , m_scene(nullptr)
    , m_context(new ViewerContext)
    , m_draw_grid(true)
    , m_initialized(false)
{

}

OpenGLScene::~OpenGLScene()
{
	delete m_camera;

	if (m_grid) {
		delete m_grid;
	}

//	if (m_scene) {
//		delete m_scene;
//	}

	delete m_context;
}

void OpenGLScene::drawBackground(QPainter *painter, QRectF &/*rect*/)
{
	std::cerr << __func__ << '\n';

	if (painter->paintEngine()->type() != QPaintEngine::OpenGL) {
		return;
	}

	if (!m_initialized) {
		initializeGL();
		m_initialized = true;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/* setup stencil mask for outlining active object */
	glStencilFunc(GL_ALWAYS, 1, 0xff);
	glStencilMask(0xff);

	m_camera->resize(width(), height());
	m_camera->update();

	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	const auto &MVP = P * MV;

	m_context->setView(m_camera->dir());
	m_context->setModelview(MV);
	m_context->setProjection(P);
	m_context->setMVP(MVP);
	m_context->setNormal(glm::inverseTranspose(glm::mat3(MV)));

	if (m_draw_grid) {
		if (!m_grid) {
			m_grid = new Grid(20, 20);
		}
		m_grid->render(MVP);
	}

	if (m_scene != nullptr) {
		m_scene->render(m_context);
	}
}

void OpenGLScene::setScene(Scene *scene)
{
	m_scene = scene;
}

void OpenGLScene::initializeGL()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
	}

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void GLWidget::initializeGL()
{
	std::cerr << __func__ << '\n';

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
	}

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}
