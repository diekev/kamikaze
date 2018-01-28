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

#include "base_editeur.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QStyle>
#include <QVariant>

BaseEditeur::BaseEditeur(QWidget *parent)
	: kangao::ConteneurControles(parent)
    , m_frame(new QFrame(this))
    , m_layout(new QHBoxLayout())
{
	QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	size_policy.setHeightForWidth(m_frame->sizePolicy().hasHeightForWidth());

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

void BaseEditeur::active(bool yesno)
{
	m_frame->setProperty("state", (yesno) ? "on" : "off");
	m_frame->setStyle(QApplication::style());
}

void BaseEditeur::set_active()
{
	if (m_context->active_widget) {
		m_context->active_widget->active(false);
	}

	m_context->active_widget = this;
	this->active(true);
}

void BaseEditeur::mousePressEvent(QMouseEvent *e)
{
	this->set_active();
	QWidget::mousePressEvent(e);
}
