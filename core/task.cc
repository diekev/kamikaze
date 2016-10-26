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

#include "task.h"

#include "ui/mainwindow.h"

/* ************************ */

TaskNotifier::TaskNotifier(MainWindow *window)
{
	if (!window) {
		return;
	}

	connect(this, SIGNAL(startTask()), window, SLOT(taskStarted()));
	connect(this, SIGNAL(updateProgress(float)), window, SLOT(updateProgress(float)));
	connect(this, SIGNAL(endTask()), window, SLOT(taskEnded()));
	connect(this, SIGNAL(nodeProcessed()), window, SLOT(nodeProcessed()));
}

void TaskNotifier::signalStart()
{
	Q_EMIT(startTask());
}

void TaskNotifier::signalProgressUpdate(float progress)
{
	Q_EMIT(updateProgress(progress));
}

void TaskNotifier::signalEnd()
{
	Q_EMIT(endTask());
}

void TaskNotifier::signalNodeProcessed()
{
	Q_EMIT(nodeProcessed());
}

/* ************************ */

Task::Task(const Context &context)
    : m_notifier(new TaskNotifier(context.main_window))
    , m_context(context)
{}

tbb::task *Task::execute()
{
	m_notifier->signalStart();

	this->start(this->m_context);

	m_notifier->signalEnd();

	return nullptr;
}
