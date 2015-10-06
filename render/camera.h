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

#pragma once

class Camera {
	int m_old_x, m_old_y;
	float m_head, m_pitch;
	float m_near, m_far, m_distance, m_fov;
	float m_zoom_speed, m_tumbling_speed, m_strafe_speed;

	glm::mat4 m_model_view, m_projection;
	glm::vec3 m_eye, m_view;
	glm::vec3 m_center, m_right, m_up;

	bool m_need_update;

public:
	Camera();
	~Camera() = default;

	void mouseMoveEvent(int button, int modifier, int x, int y);
	void mouseDownEvent(int x, int y);
	void mouseWheelEvent(int button);

	void update();
	glm::vec3 dir() const;
	glm::mat4 MV() const;
	glm::mat4 P() const;
	glm::vec3 pos() const;

	void resize(int w, int h);
	void setSpeed(const float zoom = 0.1f, const float strafe = 0.002f,
	              const float tumbling = 0.02f);
};
