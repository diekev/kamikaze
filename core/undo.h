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

#include <kamikaze/factory.h>

#include <stack>

class Context;

/* ************************************************************************** */

class Command {
protected:
	std::string m_name;

public:
	virtual ~Command() = default;

	virtual void execute(const Context &context) = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;

	void setName(const std::string &name);
	virtual bool modal() const { return false; }

	virtual void invoke(const Context &) {}
};

class CommandManager final {
	std::stack<Command *> m_undo_commands;
	std::stack<Command *> m_redo_commands;

public:
	~CommandManager();

	void execute(Command *command, const Context &context);
	void undo();
	void redo();
};

using CommandFactory = Factory<Command>;

#define REGISTER_COMMAND(factory, name, type) \
	REGISTER_TYPE(factory, name, Command, type)

/* ************************************************************************** */

struct KeyData {
	int modifier = 0;
	int key = 0;
	const char *command = nullptr;

	KeyData() = default;

	KeyData(int mod, int k, const char *cmd)
	    : modifier(mod)
	    , key(k)
	    , command(cmd)
	{}
};

void register_commands(CommandFactory *factory);

namespace KeyEventHandler {

void init_key_mappings();

void call_command(const Context &context, const KeyData &key_data, const std::string &name);

void call_modal_command(const Context &context);

void end_modal_command();

void undo();

void redo();

}  /* namespace KeyEventHandler */
