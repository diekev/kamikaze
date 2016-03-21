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

class QFrame;
class QLabel;
class QVBoxLayout;
class QGridLayout;
class Modifier;

class ModifierItem : public QWidget {
	Q_OBJECT

	QFrame *m_frame;
	QVBoxLayout *m_vlayout;
	QVBoxLayout *m_flayout;
	QGridLayout *m_layout;
	QLabel *m_label;

	Modifier *m_modifier_data;

public:
	explicit ModifierItem(const QString &name, QWidget *parent = nullptr);
	~ModifierItem();

	QGridLayout *&layout();

	void setModifierData(Modifier *modifier);

	Modifier *modifierData() const;
};
