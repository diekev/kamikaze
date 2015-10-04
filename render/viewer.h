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

#include <QGLWidget>

const int WIDTH = 1280;
const int HEIGHT = 960;

class Camera;
class Grid;
class Scene;

class Viewer : public QGLWidget {
	int m_mouse_button;
	int m_modifier;
	int m_width, m_height;
	glm::vec4 m_bg;

	Camera *m_camera;
	Grid *m_grid;
	Scene *m_scene;

public:
	Viewer(QWidget *parent);
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
	void intersectScene(int x, int y);
};
