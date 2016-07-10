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

class Camera;
class Grid;
class Scene;
class ViewerContext;

class Viewer : public QGLWidget {
	Q_OBJECT

	int m_mouse_button;
	int m_modifier;
	int m_width, m_height;
	bool m_draw_grid;
	glm::vec4 m_bg;

	Camera *m_camera;
	Grid *m_grid;
	Scene *m_scene;
	QTimer *m_timer;
	ViewerContext *m_context;

	/* Get the world space position of the given point. */
	glm::vec3 unproject(const glm::vec3 &pos) const;

public Q_SLOTS:
	void changeBackground();
	void drawGrid(bool b);

public:
	explicit Viewer(QWidget *parent = nullptr);
	~Viewer();

	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

	void keyPressEvent(QKeyEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

	void setScene(Scene *scene);

	/* Cast a ray in the scene at mouse pos (x, y). */
	void intersectScene(int x, int y) const;

	/* Select the object at screen pos (x, y). */
	void selectObject(int x, int y) const;
};

#include <QGraphicsView>

class GLWidget : public QGLWidget {
public:
	GLWidget(const QGLFormat &format)
	    : QGLWidget(format)
	{}

	void initializeGL();
};

class OpenGLScene : public QGraphicsScene {
	Q_OBJECT

	int m_mouse_button;
	int m_modifier;
	int m_width, m_height;

	Camera *m_camera;
	Grid *m_grid;
	Scene *m_scene;
	QTimer *m_timer;
	ViewerContext *m_context;

	bool m_draw_grid;
	bool m_initialized;

	/* Get the world space position of the given point. */
	glm::vec3 unproject(const glm::vec3 &pos) const;

public:
	OpenGLScene();
	~OpenGLScene();

	void drawBackground(QPainter *painter, const QRectF &rect) override;

	void keyPressEvent(QKeyEvent *e) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override;
	void mousePressEvent(QGraphicsSceneMouseEvent *e) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) override;
	void wheelEvent(QGraphicsSceneWheelEvent *e) override;

	void setScene(Scene *scene);

	/* Cast a ray in the scene at mouse pos (x, y). */
	void intersectScene(int x, int y) const;

	/* Select the object at screen pos (x, y). */
	void selectObject(int x, int y) const;

	void resize(int x, int y);

	void initializeGL();
};

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
