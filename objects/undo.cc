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

#include "undo.h"

template <typename T>
static void release_stack_memory(std::stack<T *> &stack)
{
	while (!stack.empty()) {
		auto data = stack.top();
		stack.pop();
		delete data;
	}
}

CommandManager::~CommandManager()
{
	release_stack_memory(m_undo_commands);
	release_stack_memory(m_redo_commands);
}

void CommandManager::execute(Command *command)
{
	command->execute();
	m_undo_commands.push(command);
}

void CommandManager::undo()
{
	if (m_undo_commands.empty()) {
		return;
	}

	auto command = m_undo_commands.top();
	m_undo_commands.pop();

	command->undo();

	m_redo_commands.push(command);
}

void CommandManager::redo()
{
	if (m_redo_commands.empty()) {
		return;
	}

	auto command = m_redo_commands.top();
	m_redo_commands.pop();

	command->redo();

	m_undo_commands.push(command);
}
