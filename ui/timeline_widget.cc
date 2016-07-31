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

#include "timeline_widget.h"

#include <kamikaze/context.h>

#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include "core/scene.h"

#include "utils_ui.h"

enum {
	TC_FIRST_FRAME = 0,
	TC_PLAY_BACKWARD,
	TC_PLAY_BWD_FRAME,
	TC_STOP,
	TC_PLAY_FWD_FRAME,
	TC_PLAY_FORWARD,
	TC_LAST_FRAME,

	/* Only add values above this line. */
	TC_NUM_CONTROLS,
};

static UIButData transport_controls[TC_NUM_CONTROLS] = {
    { TC_FIRST_FRAME,    "Jump To First Frame",     "icons/icon_jump_first.png" },
    { TC_PLAY_BACKWARD,  "Play Backwards",          "icons/icon_play_backward.png" },
    { TC_PLAY_BWD_FRAME, "Play Back One Frame",     "icons/icon_step_backward.png" }, // left arrow
    { TC_STOP,           "Stop",                    "icons/icon_stop.png" }, // spacebar
    { TC_PLAY_FWD_FRAME, "Play Forwards One Frame", "icons/icon_step_forward.png" }, // right arrow
    { TC_PLAY_FORWARD,   "Play Forwards",           "icons/icon_play_forward.png" }, // spacebar
    { TC_LAST_FRAME,     "Jump To Last Frame",      "icons/icon_jump_last.png" },
};

TimeLineWidget::TimeLineWidget(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
{
	m_frame = new QFrame(this);

	QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy2.setHorizontalStretch(0);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_frame->sizePolicy().hasHeightForWidth());

	m_frame->setSizePolicy(sizePolicy2);
	m_frame->setFrameShape(QFrame::StyledPanel);
	m_frame->setFrameShadow(QFrame::Raised);

	m_layout = new QHBoxLayout();
	m_vbox_layout = new QVBoxLayout(m_frame);

	m_num_layout = new QHBoxLayout();
	m_num_layout->setSizeConstraint(QLayout::SetMinimumSize);

	/* ------------------------------ jog bar ------------------------------- */

	m_slider = new QSlider(m_frame);
	m_slider->setMouseTracking(false);
	m_slider->setValue(0);
	m_slider->setOrientation(Qt::Horizontal);
	m_slider->setTickPosition(QSlider::TicksBothSides);
	m_slider->setTickInterval(0);
	m_slider->setMaximum(250);

	m_start_frame = new QSpinBox(m_frame);
	m_start_frame->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	m_start_frame->setMaximum(500000);
	m_start_frame->setValue(0);
	m_start_frame->setToolTip("Start Frame");

	m_end_frame = new QSpinBox(m_frame);
	m_end_frame->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	m_end_frame->setMaximum(500000);
	m_end_frame->setValue(250);
	m_start_frame->setToolTip("End Frame");

	m_num_layout->addWidget(m_start_frame);
	m_num_layout->addWidget(m_slider);
	m_num_layout->addWidget(m_end_frame);

	m_vbox_layout->addLayout(m_num_layout);

	/* ------------------------- current selection -------------------------- */

	m_tc_layout = new QHBoxLayout();

	m_cur_frame = new QSpinBox(m_frame);
	m_cur_frame->setAlignment(Qt::AlignCenter);
	m_cur_frame->setReadOnly(true);
	m_cur_frame->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_cur_frame->setProperty("showGroupSeparator", QVariant(false));
	m_cur_frame->setMaximum(500000);
	m_cur_frame->setToolTip("Current Frame");

	m_tc_layout->addWidget(m_cur_frame);
	m_tc_layout->addStretch();

	/* ------------------------- transport controls ------------------------- */

	for (const UIButData &transport_control : transport_controls) {
		auto button = new QPushButton(m_frame);
		button->setToolTip(transport_control.name);
		button->setIcon(QIcon(transport_control.icon_path));

		switch (transport_control.value) {
			case TC_FIRST_FRAME:
				connect(button, SIGNAL(clicked()), this, SLOT(goToStartFrame()));
				break;
			case TC_PLAY_BACKWARD:
				connect(button, SIGNAL(clicked()), this, SLOT(playBackward()));
				break;
			case TC_PLAY_BWD_FRAME:
				connect(button, SIGNAL(clicked()), this, SLOT(stepBackward()));
				break;
			case TC_STOP:
				connect(button, SIGNAL(clicked()), this, SLOT(stopAnimation()));
				break;
			case TC_PLAY_FWD_FRAME:
				connect(button, SIGNAL(clicked()), this, SLOT(stepForward()));
				break;
			case TC_PLAY_FORWARD:
				connect(button, SIGNAL(clicked()), this, SLOT(playForward()));
				break;
			case TC_LAST_FRAME:
				connect(button, SIGNAL(clicked()), this, SLOT(goToEndFrame()));
				break;
		}

		m_tc_layout->addWidget(button);
	}

	m_tc_layout->addStretch();

	/* --------------------------------- fps -------------------------------- */

	m_fps = new QDoubleSpinBox(m_frame);
	m_fps->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	m_fps->setValue(24);
	m_fps->setToolTip("Frame Rate");

	m_tc_layout->addWidget(m_fps);

	m_vbox_layout->addLayout(m_tc_layout);

	/* ------------------------- finalize ------------------------- */

	m_layout->addWidget(m_frame);

	setLayout(m_layout);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	connect(m_start_frame, SIGNAL(valueChanged(int)), this, SLOT(setStartFrame(int)));
	connect(m_end_frame, SIGNAL(valueChanged(int)), this, SLOT(setEndFrame(int)));
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(setCurrentFrame(int)));
	connect(m_fps, SIGNAL(valueChanged(double)), this, SLOT(setFPS(double)));

	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
}

void TimeLineWidget::update_state(int event_type)
{
	if (event_type != TIME_CHANGED) {
		return;
	}

	auto scene = m_context->scene;

	m_slider->setMinimum(scene->startFrame());
	m_start_frame->setValue(scene->startFrame());

	m_slider->setMaximum(scene->endFrame());
	m_end_frame->setValue(scene->endFrame());

	m_cur_frame->setValue(scene->currentFrame());

	m_slider->setValue(scene->currentFrame());

	m_fps->setValue(scene->framesPerSecond());

	/* Start or stop the animation. */
	if (m_context->animation) {
		m_timer->start(1000 / m_fps->value());
	}
	else {
		m_timer->stop();
	}
}

void TimeLineWidget::setStartFrame(int value) const
{
	auto scene = m_context->scene;
	scene->startFrame(value);
}

void TimeLineWidget::setEndFrame(int value) const
{
	auto scene = m_context->scene;
	scene->endFrame(value);
}

void TimeLineWidget::setCurrentFrame(int value) const
{
	auto scene = m_context->scene;
	scene->currentFrame(value);
}

void TimeLineWidget::setFPS(double value) const
{
	auto scene = m_context->scene;
	scene->framesPerSecond(value);
}

void TimeLineWidget::goToStartFrame() const
{
	auto scene = m_context->scene;
	scene->currentFrame(scene->startFrame());
}

void TimeLineWidget::goToEndFrame() const
{
	auto scene = m_context->scene;
	scene->currentFrame(scene->endFrame());
}

void TimeLineWidget::playForward()
{
	m_context->animation = true;
	m_context->time_direction = TIME_DIR_FORWARD;
	m_context->scene->notify_listeners(TIME_CHANGED);
}

void TimeLineWidget::playBackward()
{
	m_context->animation = true;
	m_context->time_direction = TIME_DIR_BACKWARD;
	m_context->scene->notify_listeners(TIME_CHANGED);
}

void TimeLineWidget::stepForward()
{
	m_context->time_direction = TIME_DIR_FORWARD;

	auto scene = m_context->scene;
	scene->currentFrame(scene->currentFrame() + 1);
	scene->updateForNewFrame(m_context);
}

void TimeLineWidget::stepBackward()
{
	m_context->time_direction = TIME_DIR_BACKWARD;

	auto scene = m_context->scene;
	scene->currentFrame(scene->currentFrame() - 1);
	scene->updateForNewFrame(m_context);
}

void TimeLineWidget::stopAnimation()
{
	m_context->animation = false;
	m_context->scene->notify_listeners(TIME_CHANGED);
}

void TimeLineWidget::updateFrame() const
{
	auto scene = m_context->scene;
	auto value = scene->currentFrame();

	if (m_context->time_direction == TIME_DIR_FORWARD) {
		++value;

		if (value > scene->endFrame()) {
			value = scene->startFrame();
		}
	}
	else {
		--value;

		if (value < scene->startFrame()) {
			value = scene->endFrame();
		}
	}

	scene->currentFrame(value);
	scene->updateForNewFrame(m_context);
}
