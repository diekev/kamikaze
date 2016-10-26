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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "camera.h"
#include "context.h"
#include "undo.h"

#include "util/util_input.h"

#include <kamikaze/utils_glm.h>

class CameraPanCommand : public Command {
	int m_old_x = 0;
	int m_old_y = 0;

public:
	void invoke(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		m_old_x = view_3d->x;
		m_old_y = view_3d->y;
	}

	void execute(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		const auto x = view_3d->x;
		const auto y = view_3d->y;
		const auto dx = static_cast<float>(x - m_old_x);
		const auto dy = static_cast<float>(y - m_old_y);

		const auto &camera = view_3d->camera;
		const auto &up = camera->up();
		const auto &right = camera->right();
		const auto &strafe_speed = camera->strafe_speed();

		auto center = camera->center();
		center += ((dy * up - dx * right) * strafe_speed);

		camera->center(center);

		camera->tag_update();

		m_old_x = x;
		m_old_y = y;
	}

	bool modal() const override
	{
		return true;
	}

	bool support_undo() const override
	{
		return false;
	}
};

class CameraTumbleCommand : public Command {
	int m_old_x = 0;
	int m_old_y = 0;

public:
	void invoke(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		m_old_x = view_3d->x;
		m_old_y = view_3d->y;
	}

	void execute(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		const auto x = view_3d->x;
		const auto y = view_3d->y;
		const auto dx = static_cast<float>(x - m_old_x);
		const auto dy = static_cast<float>(y - m_old_y);

		const auto &camera = view_3d->camera;
		const auto &tumbling_speed = camera->tumbling_speed();

		camera->head(camera->head() + dy * tumbling_speed);
		camera->pitch(camera->pitch() + dx * tumbling_speed);

		camera->tag_update();

		m_old_x = x;
		m_old_y = y;
	}

	bool modal() const override
	{
		return true;
	}

	bool support_undo() const override
	{
		return false;
	}
};

class CameraZoomInCommand : public Command {
public:
	void execute(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		const auto &camera = view_3d->camera;

		camera->distance(camera->distance() + camera->zoom_speed());
		camera->set_speed();
		camera->tag_update();
	}

	bool support_undo() const override
	{
		return false;
	}
};

class CameraZoomOutCommand : public Command {
public:
	void execute(const Context &context) override
	{
		const auto view_3d = context.view_3d;
		const auto &camera = view_3d->camera;

		camera->distance(glm::max(0.0f, camera->distance() - camera->zoom_speed()));
		camera->set_speed();
		camera->tag_update();
	}

	bool support_undo() const override
	{
		return false;
	}
};

void register_view3d_key_mappings(std::vector<KeyData> &keys)
{
	keys.emplace_back(MOD_KEY_NONE, MOUSE_MIDDLE, "CameraTumbleCommand");
	keys.emplace_back(MOD_KEY_SHIFT, MOUSE_MIDDLE, "CameraPanCommand");
	keys.emplace_back(MOD_KEY_NONE, MOUSE_SCROLL_DOWN, "CameraZoomOutCommand");
	keys.emplace_back(MOD_KEY_NONE, MOUSE_SCROLL_UP, "CameraZoomInCommand");
}

void register_view3d_commands(CommandFactory *factory)
{
	REGISTER_COMMAND(factory, "CameraTumbleCommand", CameraTumbleCommand);
	REGISTER_COMMAND(factory, "CameraPanCommand", CameraPanCommand);
	REGISTER_COMMAND(factory, "CameraZoomInCommand", CameraZoomInCommand);
	REGISTER_COMMAND(factory, "CameraZoomOutCommand", CameraZoomOutCommand);
}
