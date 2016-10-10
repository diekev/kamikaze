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

#include <glm/gtc/type_ptr.hpp>

class Camera {
	int m_width = 0;
	int m_height = 0;
	float m_aspect = 0.0f;
	float m_head = 30.0f;
	float m_pitch = 45.0f;
	float m_near = 0.1f;
	float m_far = 1000.0f;
	float m_distance = 25.0f;
	float m_fov = 60.0f;
	float m_zoom_speed = 0.2f;
	float m_tumbling_speed = 0.5f;
	float m_strafe_speed = 0.05f;

	glm::mat4 m_model_view = glm::mat4{0.0f};
	glm::mat4 m_projection = glm::mat4{0.0f};
	glm::vec3 m_eye = glm::vec3{0.0f, 0.0f, -1.0f};
	glm::vec3 m_view = glm::vec3{0.0f, 0.0f, 1.0f};
	glm::vec3 m_center = glm::vec3{0.0f};
	glm::vec3 m_right = glm::vec3{1.0f, 0.0f, 0.0f};
	glm::vec3 m_up = glm::vec3{0.0f, 1.0f, 0.0f};

	bool m_need_update = true;

public:
	/* No default ctor for now. */
	Camera() = delete;
	Camera(int w, int h);
	~Camera() = default;

	void update();
	glm::vec3 dir() const;
	glm::mat4 MV() const;
	glm::mat4 P() const;

	glm::vec3 pos() const;

	void resize(int w, int h);

	void set_speed(const float zoom = 0.1f,
	               const float strafe = 0.002f,
	               const float tumbling = 0.02f);

	void tag_update();

	float head() const;
	void head(float h);

	float distance() const;
	void distance(float d);

	float zoom_speed() const;
	void zoom_speed(float s);

	float strafe_speed() const;
	void strafe_speed(float s);

	float tumbling_speed() const;
	void tumbling_speed(float s);

	float pitch() const;
	void pitch(float p);

	glm::vec3 up() const;
	void up(const glm::vec3 &u);

	glm::vec3 right() const;
	void right(const glm::vec3 &r);

	glm::vec3 center() const;
	void center(const glm::vec3 &c);
};

struct View3DData {
	int x;
	int y;
	Camera *camera;
};
