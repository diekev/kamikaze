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
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QTimer>

#include "camera.h"
#include "object.h"
#include "scene.h"

#include "core/grid.h"
#include "core/manipulator.h"
#include "util/utils.h"

#include "util/utils_glm.h"

Viewer::Viewer(QWidget *parent)
    : QGLWidget(parent)
    , m_camera(new Camera(m_width, m_height))
    , m_viewer_context()
{
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

Viewer::~Viewer()
{
	delete m_camera;
	delete m_grid;
	delete m_manipulator;
}

void Viewer::initializeGL()
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
	m_manipulator = new Manipulator();
	m_camera->update();
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
			auto object = static_cast<Object *>(node.get());

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

		if (m_context->scene->currentObject() != nullptr) {
			m_manipulator->pos(m_context->scene->currentObject()->pos());

			m_viewer_context->setMatrix(m_manipulator->matrix());

			m_manipulator->render(m_viewer_context, false);

			if (m_manipulator_active) {
				glStencilFunc(GL_NOTEQUAL, 1, 0xff);
				glStencilMask(0x00);
				glDisable(GL_DEPTH_TEST);

				glLineWidth(5);
				glPolygonMode(GL_FRONT, GL_LINE);

				m_manipulator->render(m_viewer_context, true);

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
	m_base->set_active();
}

void Viewer::mouseMoveEvent(QMouseEvent *e)
{
	if (m_mouse_button == MOUSE_NONE) {
		return;
	}

	const int x = e->pos().x();
	const int y = e->pos().y();

	m_camera->mouseMoveEvent(m_mouse_button, m_modifier, x, y);

	/* move manipulator */
	if (m_context->scene->currentObject() != nullptr && m_manipulator_active) {
		const glm::vec3 start = unproject(glm::vec3(x, m_height - y, 0.0f));
		const glm::vec3 end = unproject(glm::vec3(x, m_height - y, 1.0f));

		Ray ray;
		ray.pos = m_camera->pos();
		ray.dir = end - start;

		Object *ob = m_context->scene->currentObject();
		ob->pos(m_manipulator->update(ray));
		m_context->scene->notify_listeners(OBJECT_MODIFIED);
	}

	update();
}

void Viewer::mouseReleaseEvent(QMouseEvent */*e*/)
{
	m_mouse_button = MOUSE_NONE;
	m_manipulator_active = false;
	update();
}

void Viewer::keyPressEvent(QKeyEvent *e)
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

void Viewer::wheelEvent(QWheelEvent *e)
{
	if (e->delta() < 0) {
		m_mouse_button = MOUSE_SCROLL_DOWN;
	}
	else {
		m_mouse_button = MOUSE_SCROLL_UP;
	}

	m_camera->mouseWheelEvent(m_mouse_button);
	update();
	m_base->set_active();
}

void Viewer::intersectScene(int x, int y)
{
	if (m_context->scene->currentObject() != nullptr) {
		const auto &start = unproject(glm::vec3(x, m_height - y, 0.0f));
		const auto &end = unproject(glm::vec3(x, m_height - y, 1.0f));

		Ray ray;
		ray.pos = m_camera->pos();
		ray.dir = glm::normalize(end - start);

		auto min = 1000.0f;
		if (m_manipulator->intersect(ray, min)) {
			m_manipulator_active = true;
			return;
		}
	}

	/* Figure out the position of the mouse in the scene to pick the closest
	 * object. */
	float z;
	glReadPixels(x, m_height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);

	const glm::vec3 pos = unproject(glm::vec3(x, m_height - y, z));
	m_context->scene->selectObject(pos);
}

glm::vec3 Viewer::unproject(const glm::vec3 &pos) const
{
	const auto &MV = m_camera->MV();
	const auto &P = m_camera->P();
	return glm::unProject(pos, MV, P, glm::vec4(0, 0, m_width, m_height));
}

void Viewer::changeBackground()
{
	const auto &color = QColorDialog::getColor();

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

/* ************************************************************************** */

ViewerWidget::ViewerWidget(QWidget *parent)
    : WidgetBase(parent)
    , m_viewer(new Viewer(this))
{
	m_viewer->set_base(this);

	auto vert_layout = new QVBoxLayout();
	vert_layout->addWidget(m_viewer);

	auto horiz_layout = new QHBoxLayout();

	auto push_button = new QPushButton("Change Color");
	push_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	connect(push_button, SIGNAL(clicked()), m_viewer, SLOT(changeBackground()));
	horiz_layout->addWidget(push_button);

	auto check = new QCheckBox("Draw Box");
	check->setChecked(true);
	connect(check, SIGNAL(toggled(bool)), m_viewer, SLOT(drawGrid(bool)));
	horiz_layout->addWidget(check);

	vert_layout->addLayout(horiz_layout);

	m_main_layout->addLayout(vert_layout);
}

void ViewerWidget::update_state(event_type /*event*/)
{
	m_viewer->set_context(m_context);
	m_viewer->update();
}
