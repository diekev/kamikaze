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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#pragma once

#include <QDialog>

namespace Ui {
class LevelSetDialog;
}

enum {
	ADD_LEVEL_SET_SPHERE = 0,
	ADD_LEVEL_SET_BOX    = 1,
};

class LevelSetDialog : public QDialog {
	Q_OBJECT

	Ui::LevelSetDialog *ui;

public:
	explicit LevelSetDialog(QWidget *parent = 0);
	~LevelSetDialog();

	float voxelSize() const;
	float halfWidth() const;
	float radius() const;
	int levelSetType() const;
};
