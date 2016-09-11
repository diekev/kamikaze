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

#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>  /* needs to be included before QGLWidget (includes gl.h) */
#include <QGLWidget>
#include <QGraphicsView>
#include <kamikaze/context.h>
#include <stack>

#include "widgetbase.h"
#include "util/util_input.h"

class Camera;
class Grid;
class Scene;
class ViewerContext;

/* ************************************************************************** */

class MatrixStack {
	std::stack<glm::mat4, std::vector<glm::mat4>> m_stack;

public:
	MatrixStack()
	{
		m_stack.push(glm::mat4(1.0f));
	}

	inline void push(const glm::mat4 &mat)
	{
		m_stack.push(m_stack.top() * mat);
	}

	inline void pop()
	{
		m_stack.pop();
	}

	inline const glm::mat4 &top() const
	{
		return m_stack.top();
	}
};

/* ************************************************************************** */

class OpenGLScene : public QGraphicsScene {
	Q_OBJECT

	int m_mouse_button = MOUSE_NONE;
	int m_modifier = 0;
	int m_width = 0;
	int m_height = 0;
	bool m_draw_grid = true;
	glm::vec4 m_bg = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	Camera *m_camera = nullptr;
	Grid *m_grid = nullptr;
	ViewerContext m_viewer_context;

	bool m_initialized = false;

	MatrixStack m_stack = {};

	Context *m_context = nullptr;
	WidgetBase *m_base = nullptr;

	/* Get the world space position of the given point. */
	glm::vec3 unproject(const glm::vec3 &pos) const;

public Q_SLOTS:
	void changeBackground();
	void drawGrid(bool b);

public:
	OpenGLScene();
	~OpenGLScene();

	void initializeGL();
	void resize(int w, int h);

	void drawBackground(QPainter *painter, const QRectF &rect) override;

	void keyPressEvent(QKeyEvent *e) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
	void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) override;
	void wheelEvent(QGraphicsSceneWheelEvent *e) override;

	/* Cast a ray in the scene at mouse pos (x, y). */
	void intersectScene(int x, int y) const;

	/* Select the object at screen pos (x, y). */
	void selectObject(int x, int y) const;

	void set_context(Context *context)
	{
		m_context = context;
	}

	void set_base(WidgetBase *base)
	{
		m_base = base;
	}
};

/* ************************************************************************** */

class GraphicsViewer : public QGraphicsView {
	OpenGLScene *m_glscene;

public:
	explicit GraphicsViewer(QWidget *parent = nullptr);
	~GraphicsViewer();

	OpenGLScene *GLScene() const;
	void GLScene(OpenGLScene *scene);

protected:
	void resizeEvent(QResizeEvent *event);
};

/* ************************************************************************** */

class ViewerWidget : public WidgetBase {
	Q_OBJECT

	GraphicsViewer *m_viewer;

public:
	explicit ViewerWidget(QWidget *parent = nullptr);

	void update_state(event_type event) override;
};
