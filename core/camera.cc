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
    : m_width(w)
    , m_height(h)
    , m_aspect(static_cast<float>(w) / h)
{}

void Camera::set_speed(const float zoom, const float strafe, const float tumbling)
{
	m_zoom_speed = glm::max(0.0001f, m_distance * zoom);
    m_strafe_speed = glm::max(0.0001f, m_distance * strafe);
    m_tumbling_speed = glm::max(0.2f, m_distance * tumbling);
    m_tumbling_speed = glm::min(1.0f, m_distance * tumbling);
}

void Camera::tag_update()
{
	m_need_update = true;
}

float Camera::head() const
{
	return m_head;
}

void Camera::head(float h)
{
	m_head = h;
}

float Camera::distance() const
{
	return m_distance;
}

void Camera::distance(float d)
{
	m_distance = d;
}

float Camera::zoom_speed() const
{
	return m_zoom_speed;
}

void Camera::zoom_speed(float s)
{
	m_zoom_speed = s;
}

float Camera::strafe_speed() const
{
	return m_strafe_speed;
}

void Camera::strafe_speed(float s)
{
	m_strafe_speed = s;
}

float Camera::tumbling_speed() const
{
	return m_tumbling_speed;
}

void Camera::tumbling_speed(float s)
{
	m_tumbling_speed = s;
}

float Camera::pitch() const
{
	return m_pitch;
}

void Camera::pitch(float p)
{
	m_pitch = p;
}

glm::vec3 Camera::up() const
{
	return m_up;
}

void Camera::up(const glm::vec3 &u)
{
	m_up = u;
}

glm::vec3 Camera::right() const
{
	return m_right;
}

void Camera::right(const glm::vec3 &r)
{
	m_right = r;
}

glm::vec3 Camera::center() const
{
	return m_center;
}

void Camera::center(const glm::vec3 &c)
{
	m_center = c;
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
