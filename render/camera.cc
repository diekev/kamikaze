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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <GL/glut.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

Camera::Camera()
    : m_old_x(0)
    , m_old_y(0)
    , m_rX(4.0f)
    , m_rY(50.0f)
    , m_translate(glm::vec3(0.0f, 0.0f, -2.0f))
    , m_view_rotated(false)
{}

void Camera::mouseDownEvent(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN) {
		m_old_x = x;
		m_old_y = y;
	}
	else if (s == GLUT_UP) {
		m_view_rotated = false;
	}

	if (button == 3) {
		m_translate.z += 1.0f;
	}
	else if (button == 4) {
		m_translate.z -= 1.0f;
	}
}

void Camera::mouseMoveEvent(int state, int modifier, int x, int y)
{
	if (state == 0) {
		if (modifier == 0) {
			m_rX += (y - m_old_y) / 5.0f;
			m_rY += (x - m_old_x) / 5.0f;
			m_view_rotated = true;
		}
		else if (modifier == GLUT_ACTIVE_SHIFT) {
			m_translate.x += (x - m_old_x) / 10.0f;
			m_translate.y += (y - m_old_y) / 10.0f;
		}
	}

	m_old_x = x;
	m_old_y = y;
}

void Camera::resize(int w, int h)
{
	m_projection = glm::perspective(glm::radians(60.0f), (float)w/h, 0.1f, 1000.0f);
}

void Camera::updateViewDir()
{
	glm::mat4 T = glm::translate(glm::mat4(1.0f), m_translate);
	glm::mat4 R = glm::rotate(T, glm::radians(m_rX), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model_view = glm::rotate(R, glm::radians(m_rY), glm::vec3(0.0f, 1.0f, 0.0f));
	m_view_dir = -glm::vec3(m_model_view[0][2], m_model_view[1][2], m_model_view[2][2]);
}

glm::vec3 Camera::viewDir() const
{
	return m_view_dir;
}

glm::mat4 Camera::MVP() const
{
	return m_projection * m_model_view;
}

bool Camera::hasRotated() const
{
	return m_view_rotated;
}
