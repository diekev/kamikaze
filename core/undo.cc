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

#include <cstring>

#include "context.h"

#include "util/util_input.h"
#include "util/util_memory.h"

/* ************************************************************************** */

void Command::setName(const std::string &name)
{
	m_name = name;
}

CommandManager::~CommandManager()
{
	release_stack_memory(m_undo_commands);
	release_stack_memory(m_redo_commands);
}

void CommandManager::execute(Command *command, const Context &context)
{
	command->execute(context);
	m_undo_commands.push(command);
}

static void undo_redo_ex(std::stack<Command *> &pop_stack,
                         std::stack<Command *> &push_stack,
                         bool redo)
{
	if (pop_stack.empty()) {
		return;
	}

	auto command = pop_stack.top();
	pop_stack.pop();

	if (redo) {

	}
	else {

	}

	push_stack.push(command);
}

void CommandManager::undo()
{
	undo_redo_ex(m_undo_commands, m_redo_commands, false);
}

void CommandManager::redo()
{
	undo_redo_ex(m_redo_commands, m_undo_commands, true);
}

/* ************************************************************************** */

#include "scene.h"

namespace KeyEventHandler {

#define USE_DOCUMENT

static CommandManager command_manager;
static Command *modal_command = nullptr;
static std::vector<KeyData> keys;

#ifdef USE_DOCUMENT
struct Document {
	Scene scene;
};

std::stack<Document> undo_stack;
std::stack<Document> redo_stack;

static void undo_redo(std::stack<Document> &pop_stack, std::stack<Document> &push_stack, Context &context)
{
	if (pop_stack.empty()) {
		std::cerr << "pop_stack is empty\n";
		return;
	}

	auto document = pop_stack.top();
	*context.scene = document.scene;
	pop_stack.pop();
	push_stack.push(document);
}

void undo(Context &context)
{
	undo_redo(undo_stack, redo_stack, context);
}

void redo(Context &context)
{
	undo_redo(redo_stack, undo_stack, context);
}
#endif

void init_key_mappings()
{
	register_object_key_mappings(keys);
	register_view3d_key_mappings(keys);
}

void call_command(const Context &context, const KeyData &key_data, const std::string &name)
{
	Command *command = nullptr;

	for (const KeyData &key : keys) {
		if (key.key != key_data.key) {
			continue;
		}

		if (key.modifier != key_data.modifier) {
			continue;
		}

		/* TODO */
		if (key_data.command) {
			if (std::strcmp(key.command, key_data.command) != 0) {
				continue;
			}
		}

		if (!context.command_factory->registered(key.command)) {
			continue;
		}

		command = (*context.command_factory)(key.command);
		break;
	}

	if (command == nullptr) {
		return;
	}

	/* TODO: find a way to pass custom datas. */
	command->setName(name);

#ifdef USE_DOCUMENT
	if (command->support_undo()) {
		Document doc;
		doc.scene = *context.scene;

		std::cerr << "Pushing scene to undo stack\n";
		undo_stack.push(doc);
	}
#endif

	if (command->modal()) {
		modal_command = command;
		modal_command->invoke(context);
	}
	else {
		command->execute(context);
	}
}

void call_modal_command(const Context &context)
{
	if (modal_command == nullptr) {
		return;
	}

	modal_command->execute(context);
}

void end_modal_command()
{
	delete modal_command;
	modal_command = nullptr;
}

#ifndef USE_DOCUMENT
void undo()
{
	/* TODO: figure out how to update everything properly */
	command_manager.undo();
}

void redo()
{
	/* TODO: figure out how to update everything properly */
	command_manager.redo();
}
#endif

}  /* namespace KeyEventHandler */
