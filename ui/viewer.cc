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
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>

#include "camera.h"
#include "object.h"
#include "scene.h"

#include "grid.h"
#include "util/util_input.h"

/* ************************************************************************** */

OpenGLScene::OpenGLScene()
    : m_mouse_button(MOUSE_NONE)
    , m_width(0)
    , m_height(0)
    , m_draw_grid(true)
    , m_bg(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
    , m_camera(new Camera(m_width, m_height))
    , m_grid(nullptr)
    , m_scene(nullptr)
    , m_context(new ViewerContext)
    , m_initialized(false)
{}

OpenGLScene::~OpenGLScene()
{
	delete m_camera;
	delete m_grid;
	delete m_context;
}

void OpenGLScene::initializeGL()
{
	glewExperimental = GL_TRUE;
	const auto &err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
	}

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void OpenGLScene::resize(int x, int y)
{
	setSceneRect(0, 0, x, y);
	m_camera->resize(x, y);
	m_width = x;
	m_height = y;
}

void OpenGLScene::drawBackground(QPainter *painter, const QRectF &/*rect*/)
{
	if (painter->paintEngine()->type() != QPaintEngine::OpenGL &&
	    painter->paintEngine()->type() != QPaintEngine::OpenGL2)
	{
		std::cerr << __func__ << ": needs a QGLWidget to be set as viewport on the graphics view\n";
		return;
	}

	if (!m_initialized) {
		initializeGL();
		m_initialized = true;
	}

	glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
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
	m_context->setMatrix(glm::mat4(1.0f));

	if (m_draw_grid) {
		if (!m_grid) {
			m_grid = new Grid(20, 20);
		}

		m_grid->render(m_context);
	}

	if (m_scene != nullptr) {
		for (auto &object : m_scene->objects()) {
			if (!object || !object->collection()) {
				continue;
			}

			const bool active_object = (object == m_scene->currentObject());

			const auto collection = object->collection();

			for (auto &prim : collection->primitives()) {
				/* update prim before drawing */
				prim->update();
				prim->prepareRenderData();

				if (prim->drawBBox()) {
					prim->bbox()->render(m_context, false);
				}

				m_context->setMatrix(object->matrix() * prim->matrix());

				prim->render(m_context, false);

				if (active_object) {
					glStencilFunc(GL_NOTEQUAL, 1, 0xff);
					glStencilMask(0x00);
					glDisable(GL_DEPTH_TEST);

					glLineWidth(5);
					glPolygonMode(GL_FRONT, GL_LINE);

					prim->render(m_context, true);

					/* Restore state. */
					glPolygonMode(GL_FRONT, GL_FILL);
					glLineWidth(1);

					glStencilFunc(GL_ALWAYS, 1, 0xff);
					glStencilMask(0xff);
					glEnable(GL_DEPTH_TEST);
				}
			}
		}
	}

	QTimer::singleShot(40, this, SLOT(update()));
}

void OpenGLScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	const int x = e->scenePos().x();
	const int y = e->scenePos().y();

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

void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if (m_mouse_button == MOUSE_NONE) {
		return;
	}

	const int x = e->scenePos().x();
	const int y = e->scenePos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
}

void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent */*e*/)
{
	m_mouse_button = MOUSE_NONE;
}

void OpenGLScene::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
		case Qt::Key_Delete:
			m_scene->removeObject(m_scene->currentObject());
			break;
		default:
			break;
	}
}

void OpenGLScene::wheelEvent(QGraphicsSceneWheelEvent *e)
{
	if (e->delta() < 0) {
		m_mouse_button = MOUSE_SCROLL_DOWN;
	}
	else {
		m_mouse_button = MOUSE_SCROLL_UP;
	}

	m_camera->mouseWheelEvent(m_mouse_button);
}

void OpenGLScene::setScene(Scene *scene)
{
	m_scene = scene;
}

void OpenGLScene::intersectScene(int x, int y) const
{
	const auto &start = unproject(glm::vec3(x, m_height - y, 0.0f));
	const auto &end = unproject(glm::vec3(x, m_height - y, 1.0f));

	Ray ray;
	ray.pos = m_camera->pos();
	ray.dir = glm::normalize(end - start);

	m_scene->intersect(ray);
}

void OpenGLScene::selectObject(int x, int y) const
{
	float z;
	glReadPixels(x, m_height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

	const glm::vec3 pos = unproject(glm::vec3(x, m_height - y, z));
	m_scene->selectObject(pos);
}

glm::vec3 OpenGLScene::unproject(const glm::vec3 &pos) const
{
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	return glm::unProject(pos, MV, P, glm::vec4(0, 0, m_width, m_height));
}

void OpenGLScene::changeBackground()
{
	const auto &color = QColorDialog::getColor();

	if (color.isValid()) {
		m_bg = glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.0f);
		glClearColor(m_bg.r, m_bg.g, m_bg.b, m_bg.a);
	}
}

void OpenGLScene::drawGrid(bool b)
{
	m_draw_grid = b;
}

/* ************************************************************************** */

#include <QGraphicsTextItem>
#include <QLabel>
#include <QVBoxLayout>

QDialog *createDialog(const QString &windowTitle)
{
	QDialog *dialog = new QDialog(nullptr, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

	dialog->setWindowOpacity(0.8);
	dialog->setWindowTitle(windowTitle);
	dialog->setLayout(new QVBoxLayout);

	return dialog;
}

GraphicsViewer::GraphicsViewer(QWidget *parent)
    : QGraphicsView(parent)
{
	GLScene(new OpenGLScene);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	QWidget *instructions = createDialog(tr("Instructions"));
     instructions->layout()->addWidget(new QLabel(
         tr("Use mouse wheel to zoom model, and click and "
            "drag to rotate model")));
     instructions->layout()->addWidget(new QLabel(
         tr("Move the sun around to change the light "
            "position")));

     m_glscene->addWidget(instructions);

	 for (auto items : m_glscene->items()) {
		 std::cerr << "There is an item\n";
		 items->setPos(250, 250);
	 }
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
		OpenGLScene *glscene = static_cast<OpenGLScene *>(scene());
		glscene->resize(event->size().width(), event->size().height());

		QGraphicsView::resizeEvent(event);
	}
}
