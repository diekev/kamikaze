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

#pragma once

#include <kamikaze/context.h>
#include <kamikaze/primitive.h>

class EvaluationContext;
class FenetrePrincipale;
class Scene;
class UsineOperateur;
class BaseEditeur;

/* - 0x000000ff Category.
 * - 0x0000ff00 Action.
 */

enum class event_type {
	/* Category. */
	object = (1 << 0),
	node   = (2 << 0),
	time   = (3 << 0),

	/* Action. */
	added     = (1 << 8),
	removed   = (2 << 8),
	selected  = (3 << 8),
	modified  = (4 << 8),
	parented  = (5 << 8),
	processed = (6 << 8),
};

constexpr event_type operator&(event_type lhs, event_type rhs)
{
	return static_cast<event_type>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

constexpr event_type operator&(event_type lhs, int rhs)
{
	return static_cast<event_type>(static_cast<int>(lhs) & rhs);
}

constexpr event_type operator|(event_type lhs, event_type rhs)
{
	return static_cast<event_type>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr event_type operator^(event_type lhs, event_type rhs)
{
	return static_cast<event_type>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

constexpr event_type operator~(event_type lhs)
{
	return static_cast<event_type>(~static_cast<int>(lhs));
}

event_type &operator|=(event_type &lhs, event_type rhs);
event_type &operator&=(event_type &lhs, event_type rhs);
event_type &operator^=(event_type &lhs, event_type rhs);

constexpr auto get_action(event_type etype)
{
	return etype & 0x0000ff00;
}

constexpr auto get_category(event_type etype)
{
	return etype & 0x000000ff;
}

template <typename char_type>
std::basic_ostream<char_type> &operator<<(std::basic_ostream<char_type> &os, event_type event)
{
	switch (get_category(event)) {
		case event_type::object:
			os << "object, ";
			break;
		case event_type::node:
			os << "node, ";
			break;
		case event_type::time:
			os << "time, ";
			break;
		default:
			break;
	}

	switch (get_action(event)) {
		case event_type::added:
			os << "added";
			break;
		case event_type::modified:
			os << "modified";
			break;
		case event_type::parented:
			os << "parented";
			break;
		case event_type::removed:
			os << "removed";
			break;
		case event_type::selected:
			os << "selected";
			break;
		case event_type::processed:
			os << "processed";
			break;
		default:
			break;
	}

	return os;
}

class ContextListener {
protected:
	Context *m_context = nullptr;

public:
	virtual ~ContextListener();

	void listens(Context *ctx);

	virtual void update_state(event_type event) = 0;
};

class Listened {
	std::vector<ContextListener *> m_listeners;

public:
	void add_listener(ContextListener *listener);

	void remove_listener(ContextListener *listener);

	void notify_listeners(event_type event);
};
