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

#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "util/util_input.h"

Camera::Camera(int w, int h)
    : m_old_x(0)
    , m_old_y(0)
    , m_width(w)
    , m_height(h)
    , m_aspect(float(w) / h)
    , m_head(30.0f)
    , m_pitch(45.0f)
    , m_near(0.1f)
    , m_far(1000.0f)
    , m_distance(25.0f)
    , m_fov(60.0f)
    , m_zoom_speed(0.2f)
    , m_tumbling_speed(0.5f)
    , m_strafe_speed(0.05f)
    , m_eye(glm::vec3(0.0f, 0.0f, -1.0f))
    , m_view(glm::vec3(0.0f, 0.0f, 1.0f))
    , m_center(glm::vec3(0.0f))
    , m_right(glm::vec3(1.0f, 0.0f, 0.0f))
    , m_up(glm::vec3(0.0f, 1.0f, 0.0f))
    , m_need_update(true)
{}

void Camera::setSpeed(const float zoom, const float strafe, const float tumbling)
{
	m_zoom_speed = glm::max(0.0001f, m_distance * zoom);
    m_strafe_speed = glm::max(0.0001f, m_distance * strafe);
    m_tumbling_speed = glm::max(0.2f, m_distance * tumbling);
    m_tumbling_speed = glm::min(1.0f, m_distance * tumbling);
}

void Camera::mouseDownEvent(int x, int y)
{
	m_old_x = x;
	m_old_y = y;
}

void Camera::mouseWheelEvent(int button)
{
	if (button == MOUSE_SCROLL_UP) {
		m_distance += m_zoom_speed;
	}
	else if (button == MOUSE_SCROLL_DOWN) {
		const float temp = m_distance - m_zoom_speed;
		m_distance = glm::max(0.0f, temp);
	}

	setSpeed();

	m_need_update = true;
}

void Camera::mouseMoveEvent(int button, int modifier, int x, int y)
{
	const float dx = (x - m_old_x);
	const float dy = (y - m_old_y);

	if (button == MOUSE_MIDDLE) {
		if (modifier == MOD_KEY_NONE) {
			m_head += dy * m_tumbling_speed;
	        m_pitch += dx * m_tumbling_speed;
		}
		else if (modifier == MOD_KEY_SHIFT) {
			m_center += (dy * m_up - dx * m_right) * m_strafe_speed;
		}
	}

	m_old_x = x;
	m_old_y = y;

	m_need_update = true;
}

void Camera::resize(int w, int h)
{
	m_width = w;
	m_height = h;
	m_aspect = float(w) / h;

	m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
	m_need_update = true;
}

void Camera::update()
{
	if (!m_need_update) {
		return;
	}

	const float head = glm::radians(m_head);
	const float pitch = glm::radians(m_pitch);

	m_eye[0] = m_center[0] + m_distance * std::cos(head) * std::cos(pitch);
	m_eye[1] = m_center[1] + m_distance * std::sin(head);
	m_eye[2] = m_center[2] + m_distance * std::cos(head) * std::sin(pitch);

	m_view = glm::normalize(m_center - m_eye);

	m_up[1] = (glm::cos(head)) > 0 ? 1.0f : -1.0f;
	m_right = glm::cross(m_view, m_up);

	m_model_view = glm::lookAt(m_eye, m_center, m_up);
	m_need_update = false;
}

glm::vec3 Camera::dir() const
{
	return m_view;
}

glm::mat4 Camera::MV() const
{
	return m_model_view;
}

glm::mat4 Camera::P() const
{
	return m_projection;
}

glm::vec3 Camera::pos() const
{
	glm::mat4 view_model = glm::inverse(m_model_view);
	return glm::vec3(view_model[3]);
}
