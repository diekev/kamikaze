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
	m_hbox_layout = new QHBoxLayout(m_frame);
	m_slider = new QSlider(m_frame);
	m_slider->setMouseTracking(false);
	m_slider->setValue(0);
	m_slider->setOrientation(Qt::Horizontal);
	m_slider->setTickPosition(QSlider::TicksBothSides);
	m_slider->setTickInterval(0);
	m_slider->setMaximum(250);

	m_hbox_layout->addWidget(m_slider);

	m_grid_layout = new QGridLayout();
	m_grid_layout->setSizeConstraint(QLayout::SetMinimumSize);

	m_end_frame = new QSpinBox(m_frame);
	m_end_frame->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	m_end_frame->setMaximum(500000);
	m_end_frame->setValue(250);

	m_grid_layout->addWidget(m_end_frame, 1, 2, 1, 1);

	m_fps = new QDoubleSpinBox(m_frame);
	m_fps->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	m_fps->setValue(24);

	m_grid_layout->addWidget(m_fps, 2, 2, 1, 1);

	m_start_frame = new QSpinBox(m_frame);
	m_start_frame->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	m_start_frame->setMaximum(500000);

	m_grid_layout->addWidget(m_start_frame, 1, 0, 1, 1);

	m_begin_but = new QPushButton(m_frame);

	m_grid_layout->addWidget(m_begin_but, 0, 0, 1, 1);

	m_cur_frame = new QSpinBox(m_frame);
	m_cur_frame->setAlignment(Qt::AlignCenter);
	m_cur_frame->setReadOnly(true);
	m_cur_frame->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_cur_frame->setProperty("showGroupSeparator", QVariant(false));
	m_cur_frame->setMaximum(500000);

	m_grid_layout->addWidget(m_cur_frame, 1, 1, 1, 1);

	m_start_but = new QPushButton(m_frame);
	m_grid_layout->addWidget(m_start_but, 0, 1, 1, 1);

	m_end_but = new QPushButton(m_frame);
	m_grid_layout->addWidget(m_end_but, 0, 2, 1, 1);

	m_label = new QLabel(m_frame);
	m_grid_layout->addWidget(m_label, 2, 1, 1, 1);

	m_hbox_layout->addLayout(m_grid_layout);

	m_start_but->setIcon(QIcon("icons/icon_play_forward.png"));
	m_begin_but->setIcon(QIcon("icons/icon_jump_first.png"));
	m_end_but->setIcon(QIcon("icons/icon_jump_last.png"));

	m_layout->addWidget(m_frame);
	setLayout(m_layout);

	connect(m_start_frame, SIGNAL(valueChanged(int)), this, SLOT(setStartFrame(int)));
	connect(m_end_frame, SIGNAL(valueChanged(int)), this, SLOT(setEndFrame(int)));
	connect(m_slider, SIGNAL(valueChanged(int)), m_cur_frame, SLOT(setValue(int)));
	connect(m_fps, SIGNAL(valueChanged(double)), this, SLOT(setFPS(double)));

	connect(m_begin_but, SIGNAL(clicked()), this, SLOT(goToStartFrame()));
	connect(m_start_but, SIGNAL(clicked()), this, SLOT(startAnimation()));
	connect(m_end_but, SIGNAL(clicked()), this, SLOT(goToEndFrame()));

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

	m_slider->setValue(scene->currentFrame());

	m_fps->setValue(scene->framesPerSecond());

	/* Start or stop the animation. */
	if (m_context->animation) {
		m_start_but->setIcon(QIcon("icons/icon_pause.png"));
		m_timer->start(1000 / m_fps->value());
	}
	else {
		m_start_but->setIcon(QIcon("icons/icon_play_forward.png"));
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

void TimeLineWidget::startAnimation()
{
	m_context->animation = !m_context->animation;
	m_context->scene->notify_listeners(TIME_CHANGED);
}

void TimeLineWidget::updateFrame() const
{
	auto scene = m_context->scene;
	auto value = scene->currentFrame() + 1;

	if (value == scene->endFrame()) {
		value = 0;
	}

	scene->currentFrame(value);
	scene->updateForNewFrame(m_context);
}
