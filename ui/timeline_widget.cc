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
#include <kangao/kangao.h>
#include <kangao/manipulable.h>

#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>

#include "core/scene.h"

TimeLineWidget::TimeLineWidget(kangao::RepondantBouton *repondant, QWidget *parent)
    : WidgetBase(parent)
    , m_timer(new QTimer(this))
{
	m_vbox_layout = new QVBoxLayout();
	m_main_layout->addLayout(m_vbox_layout);

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
	m_end_frame->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
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

	/* ------------------------- transport controls ------------------------- */

	m_tc_layout->addStretch();

	kangao::Manipulable dummy;

	kangao::DonneesInterface donnees;
	donnees.conteneur = nullptr;
	donnees.manipulable = &dummy;
	donnees.repondant_bouton = repondant;

	auto text_entree = kangao::contenu_fichier("interface/disposition_ligne_temps.kangao");
	auto disp_controles = kangao::compile_interface(donnees, text_entree.c_str());

	m_tc_layout->addLayout(disp_controles);

	m_tc_layout->addStretch();

	/* --------------------------------- fps -------------------------------- */

	m_fps = new QDoubleSpinBox(m_frame);
	m_fps->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
	m_fps->setValue(24);
	m_fps->setToolTip("Frame Rate");

	m_tc_layout->addWidget(m_fps);

	m_vbox_layout->addLayout(m_tc_layout);

	/* ------------------------------ finalize ------------------------------ */

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	connect(m_start_frame, SIGNAL(valueChanged(int)), this, SLOT(setStartFrame(int)));
	connect(m_end_frame, SIGNAL(valueChanged(int)), this, SLOT(setEndFrame(int)));
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(setCurrentFrame(int)));
	connect(m_fps, SIGNAL(valueChanged(double)), this, SLOT(setFPS(double)));

	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
}

void TimeLineWidget::update_state(event_type event)
{
	if (event != (event_type::time | event_type::modified)) {
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
	if (m_context->eval_ctx->animation) {
		m_timer->start(1000 / m_fps->value());
	}
	else {
		m_timer->stop();
	}
}

void TimeLineWidget::setStartFrame(int value)
{
	this->set_active();
	auto scene = m_context->scene;
	scene->startFrame(value);
}

void TimeLineWidget::setEndFrame(int value)
{
	this->set_active();
	auto scene = m_context->scene;
	scene->endFrame(value);
}

void TimeLineWidget::setCurrentFrame(int value)
{
	this->set_active();
	auto scene = m_context->scene;
	scene->currentFrame(value);
	scene->updateForNewFrame(*m_context);
}

void TimeLineWidget::setFPS(double value)
{
	this->set_active();
	auto scene = m_context->scene;
	scene->framesPerSecond(value);
}

void TimeLineWidget::updateFrame() const
{
	auto scene = m_context->scene;
	auto value = scene->currentFrame();

	if (m_context->eval_ctx->time_direction == TIME_DIR_FORWARD) {
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
	scene->updateForNewFrame(*m_context);
}
