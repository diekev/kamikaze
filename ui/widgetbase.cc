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

#include "widgetbase.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QVariant>

#include "scene.h"

WidgetBase::WidgetBase(Context &context, QWidget *parent)
    : QWidget(parent)
    , m_frame(new QFrame(this))
    , m_layout(new QVBoxLayout())
{
	this->listens(&context);

	QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	size_policy.setHeightForWidth(m_frame->sizePolicy().hasHeightForWidth());

	/* Nagivation bar. */

	auto navigation_layout = new QHBoxLayout();

	auto prev = new QPushButton(QIcon("icons/icon_nav_prev.png"), "");
	prev->setToolTip("Go to previous node in the history");
	connect(prev, SIGNAL(clicked()), this, SLOT(previous_node()));

	auto up = new QPushButton(QIcon("icons/icon_nav_parent.png"), "");
	up->setToolTip("Go to parent node in the tree");
	connect(up, SIGNAL(clicked()), this, SLOT(parent_node()));

	auto next = new QPushButton(QIcon("icons/icon_nav_next.png"), "");
	next->setToolTip("Go to next node in the history");
	connect(next, SIGNAL(clicked()), this, SLOT(next_node()));

	auto home = new QPushButton(QIcon("icons/icon_nav_home.png"), "");
	home->setToolTip("Go to root node in the hierarchy");
	connect(home, SIGNAL(clicked()), this, SLOT(root_node()));

	auto reload = new QPushButton(QIcon("icons/icon_nav_reload.png"), "");
	reload->setToolTip("Refresh current view");
	connect(reload, SIGNAL(clicked()), this, SLOT(reload_view()));

	auto line_edit = new QLineEdit();
	line_edit->setText(m_context->scene->current_node()->get_dag_path().c_str());

	navigation_layout->addWidget(prev);
	navigation_layout->addWidget(up);
	navigation_layout->addWidget(next);
	navigation_layout->addWidget(reload);
	navigation_layout->addWidget(home);
	navigation_layout->addWidget(line_edit);

	m_layout->addLayout(navigation_layout);

	/* Intern frame, where individual interface regions put their buttons. */

	m_frame->setSizePolicy(size_policy);
	m_frame->setFrameShape(QFrame::StyledPanel);
	m_frame->setFrameShadow(QFrame::Raised);

	m_layout->addWidget(m_frame);

	m_layout->setMargin(0);
	this->setLayout(m_layout);

	m_main_layout = new QHBoxLayout(m_frame);
	m_main_layout->setMargin(6);

	this->active(false);
}

void WidgetBase::active(bool yesno)
{
	m_frame->setProperty("state", (yesno) ? "on" : "off");
	m_frame->setStyle(QApplication::style());
}

void WidgetBase::set_active()
{
	if (m_context->active_widget) {
		m_context->active_widget->active(false);
	}

	m_context->active_widget = this;
	this->active(true);
}

void WidgetBase::mousePressEvent(QMouseEvent *e)
{
	this->set_active();
	QWidget::mousePressEvent(e);
}

void WidgetBase::previous_node()
{
	/* TODO */
	this->update_state(event_type::refresh);
}

void WidgetBase::next_node()
{
	/* TODO */
	this->update_state(event_type::refresh);
}

void WidgetBase::parent_node()
{
	auto scene = m_context->scene;
	auto current_node = scene->current_node();

	if (current_node != scene->root_node()) {
		scene->current_node(current_node->parent());
	}

	this->update_state(event_type::refresh);
}

void WidgetBase::root_node()
{
	auto scene = m_context->scene;
	scene->current_node(scene->root_node());

	this->update_state(event_type::refresh);
}

void WidgetBase::reload_view()
{
	this->update_state(event_type::refresh);
}
