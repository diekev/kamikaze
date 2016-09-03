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

#include "context.h"

#include <algorithm>
#include <kamikaze/context.h>

#include "scene.h"

event_type &operator&=(event_type &lhs, event_type rhs)
{
	return (lhs = lhs & rhs);
}

event_type &operator|=(event_type &lhs, event_type rhs)
{
	return (lhs = lhs | rhs);
}

event_type &operator^=(event_type &lhs, event_type rhs)
{
	return (lhs = lhs ^ rhs);
}

ContextListener::~ContextListener()
{
	if (m_context->scene) {
		m_context->scene->remove_listener(this);
	}
}

void ContextListener::listens(Context *ctx)
{
	m_context = ctx;
	m_context->scene->add_listener(this);
}

void Listened::add_listener(ContextListener *listener)
{
	m_listeners.push_back(listener);
}

void Listened::remove_listener(ContextListener *listener)
{
	auto iter = std::find(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(iter);
}

void Listened::notify_listeners(event_type event)
{
	for (auto &listener : m_listeners) {
		listener->update_state(event);
	}
}
