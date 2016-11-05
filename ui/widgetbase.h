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

#include <QWidget>

#include "context.h"

class QFrame;
class QHBoxLayout;
class QVBoxLayout;

class WidgetBase : public QWidget, public ContextListener {
	Q_OBJECT

protected:
	QFrame *m_frame;
	QVBoxLayout *m_layout;
	QHBoxLayout *m_main_layout;

public:
	explicit WidgetBase(Context &context, QWidget *parent = nullptr);
	virtual ~WidgetBase() = default;

	void active(bool yesno);
	void set_active();

	void mousePressEvent(QMouseEvent *e) override;

public Q_SLOTS:
	void previous_node();
	void next_node();
	void parent_node();
	void root_node();
	void reload_view();
};
