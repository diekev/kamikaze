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
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#pragma once

#include <QWidget>

class TimeLineWidget final : public QWidget {
	Q_OBJECT

	int m_start_frame, m_end_frame, m_current_frame;
	bool m_mouse_pressed;

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);

Q_SIGNALS:
	void currentFrameChanged(int frame);

public Q_SLOTS:
	void setStartFrame(const int start);
	void setEndFrame(const int end);

public:
	explicit TimeLineWidget(QWidget *parent = nullptr);
	~TimeLineWidget() = default;

	void setCurrentFrame(const int frame);
	void updateCurrentFrame(const int pos_x);
	void incrementFrame();
};
