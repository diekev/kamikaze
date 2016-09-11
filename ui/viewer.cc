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
#include <kamikaze/renderbuffer.h>

#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QFrame>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>

#include "camera.h"
#include "object.h"
#include "scene.h"

#include "grid.h"

/* ************************************************************************** */

OpenGLScene::OpenGLScene()
    : m_camera(new Camera(0, 0))
    , m_viewer_context(new ViewerContext)
{}

OpenGLScene::~OpenGLScene()
{
	delete m_camera;
	delete m_grid;
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

	m_grid = new Grid(20, 20);
	m_camera->update();
}

void OpenGLScene::resize(int w, int h)
{
	setSceneRect(0, 0, w, h);
	m_camera->resize(w, h);
	m_width = w;
	m_height = h;
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

	/* Make sure buffers are freed in the main thread. */
	purge_all_buffers();

	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	const auto &MVP = P * MV;

	m_viewer_context.setView(m_camera->dir());
	m_viewer_context.setModelview(MV);
	m_viewer_context.setProjection(P);
	m_viewer_context.setMVP(MVP);
	m_viewer_context.setNormal(glm::inverseTranspose(glm::mat3(MV)));
	m_viewer_context.setMatrix(m_stack.top());
	m_viewer_context.for_outline(false);

	if (m_draw_grid) {
		m_grid->render(m_viewer_context);
	}

	/* XXX - only happens on initialization, but not nice. Better to construct
	 * context listeners with valid context. */
	if (!m_context) {
		return;
	}

	if (m_context->scene != nullptr) {
		for (auto &node : m_context->scene->nodes()) {
			auto object = static_cast<Object *>(node);

			if (!object->collection()) {
				continue;
			}

			const bool active_object = (object == m_context->scene->active_node());

			const auto collection = object->collection();

			if (object->parent()) {
				m_stack.push(object->parent()->matrix());
			}

			m_stack.push(object->matrix());

			for (auto &prim : collection->primitives()) {
				/* update prim before drawing */
				prim->update();
				prim->prepareRenderData();

				if (prim->drawBBox()) {
					prim->bbox()->render(m_viewer_context);
				}

				m_stack.push(prim->matrix());

				m_viewer_context.setMatrix(m_stack.top());

				prim->render(m_viewer_context);

				if (active_object) {
					m_viewer_context.for_outline(true);

					glStencilFunc(GL_NOTEQUAL, 1, 0xff);
					glStencilMask(0x00);
					glDisable(GL_DEPTH_TEST);

					glLineWidth(5);
					glPolygonMode(GL_FRONT, GL_LINE);

					prim->render(m_viewer_context);

					/* Restore state. */
					glPolygonMode(GL_FRONT, GL_FILL);
					glLineWidth(1);

					glStencilFunc(GL_ALWAYS, 1, 0xff);
					glStencilMask(0xff);
					glEnable(GL_DEPTH_TEST);

					m_viewer_context.for_outline(false);
				}

				m_stack.pop();
			}

			m_stack.pop();

			if (object->parent()) {
				m_stack.pop();
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
	update();
	m_base->set_active();
}

void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
	if (m_mouse_button == MOUSE_NONE) {
		return;
	}

	const int x = e->scenePos().x();
	const int y = e->scenePos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);
	update();
}

void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent */*e*/)
{
	m_mouse_button = MOUSE_NONE;
}

void OpenGLScene::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
		case Qt::Key_Delete:
			m_context->scene->removeObject(m_context->scene->active_node());
			break;
		default:
			break;
	}

	m_base->set_active();
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

void OpenGLScene::intersectScene(int x, int y) const
{
	const auto &start = unproject(glm::vec3(x, m_height - y, 0.0f));
	const auto &end = unproject(glm::vec3(x, m_height - y, 1.0f));

	Ray ray;
	ray.pos = m_camera->pos();
	ray.dir = glm::normalize(end - start);

	m_context->scene->intersect(ray);
}

void OpenGLScene::selectObject(int x, int y) const
{
	float z;
	glReadPixels(x, m_height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

	const glm::vec3 pos = unproject(glm::vec3(x, m_height - y, z));
	m_context->scene->selectObject(pos);
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
		update();
	}
}

void OpenGLScene::drawGrid(bool b)
{
	m_draw_grid = b;
	update();
}

/* ************************************************************************** */

#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QVBoxLayout>

GraphicsViewer::GraphicsViewer(QWidget *parent)
    : QGraphicsView(parent)
{
	GLScene(new OpenGLScene());

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	QLabel *item = new QLabel("Use mouse wheel to zoom model, and click and drag to rotate model", this);
	item->show();

	for (auto items : scene()->items()) {
		items->show();
		items->setPos(0, 0);
		std::cerr << "There is an item at <" << items->pos().x() << ", " << items->pos().y() << ">\n";
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

/* ************************************************************************** */

ViewerWidget::ViewerWidget(QWidget *parent)
    : WidgetBase(parent)
    , m_viewer(new GraphicsViewer(this))
{
	auto scene = m_viewer->GLScene();

	scene->set_base(this);

	auto vert_layout = new QVBoxLayout();
	vert_layout->addWidget(m_viewer);

	auto horiz_layout = new QHBoxLayout();

	auto push_button = new QPushButton("Change Color");
	push_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	connect(push_button, SIGNAL(clicked()), scene, SLOT(changeBackground()));
	horiz_layout->addWidget(push_button);

	auto check = new QCheckBox("Draw Box");
	check->setChecked(true);
	connect(check, SIGNAL(toggled(bool)), scene, SLOT(drawGrid(bool)));
	horiz_layout->addWidget(check);

	vert_layout->addLayout(horiz_layout);

	m_main_layout->addLayout(vert_layout);
}

void ViewerWidget::update_state(event_type /*event*/)
{
	m_viewer->GLScene()->set_context(m_context);
	m_viewer->update();
}
