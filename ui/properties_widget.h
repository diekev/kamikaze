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

class Persona;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QScrollArea;

class PropertiesWidget : public QWidget, public ContextListener {
	Q_OBJECT

	QWidget *m_widget;
	QScrollArea *m_scroll;
	QGridLayout *m_layout;
	QHBoxLayout *m_hbox_layout;

public:
	explicit PropertiesWidget(QWidget *parent = nullptr);

	void update_state(int event_type) override;

private:
	void drawProperties(Persona *persona, bool set_context);

private Q_SLOTS:
	void evalObjectGraph();
	void tagObjectUpdate();
	void updateProperties();
};
