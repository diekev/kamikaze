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

class QDoubleSpinBox;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QSlider;
class QPushButton;
class QSpinBox;
class QVBoxLayout;
class Scene;

class TimeLineWidget : public QWidget, public ContextListener {
	Q_OBJECT

	QFrame *m_frame;
	QSlider *m_slider;

	QHBoxLayout *m_layout;
	QHBoxLayout *m_tc_layout;
	QHBoxLayout *m_num_layout;
	QVBoxLayout *m_vbox_layout;
	QSpinBox *m_end_frame, *m_start_frame, *m_cur_frame;
	QDoubleSpinBox *m_fps;

	QTimer *m_timer;

	bool m_timer_has_started = false;

public:
	explicit TimeLineWidget(QWidget *parent = nullptr);

	void update_state(event_type event) override;

private Q_SLOTS:
	void setStartFrame(int value) const;
	void setCurrentFrame(int value) const;
	void setEndFrame(int value) const;
	void setFPS(double value) const;

	void goToStartFrame() const;
	void goToEndFrame() const;

	void playBackward();
	void playForward();
	void stopAnimation();
	void stepBackward();
	void stepForward();

	void updateFrame() const;
};
