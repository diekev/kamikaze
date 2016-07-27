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

#include <memory>
#include <QObject>
#include <tbb/task.h>

class EvaluationContext;
class MainWindow;

/* Apparently we can not have a class derived from both a QObject and a
 * tbb::task so this class is to be used in conjunction with a tbb::task derived
 * class to notify the UI about certain events.
 */
class TaskNotifier : public QObject {
	Q_OBJECT

public:
	explicit TaskNotifier(MainWindow *window);

	void signalStart();
	void signalProgressUpdate(float progress);
	void signalEnd();

Q_SIGNALS:
	void startTask();
	void updateProgress(float progress);
	void endTask();
};

class Task : public tbb::task {
protected:
	std::unique_ptr<TaskNotifier> m_notifier = nullptr;
	const EvaluationContext *m_context = nullptr;

public:
	Task(const EvaluationContext * const context);

	tbb::task *execute() override;

	virtual void start(const EvaluationContext * const context) = 0;
};
