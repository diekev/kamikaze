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

#include "modifieritem.h"

#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>

ModifierItem::ModifierItem(const QString &name, QWidget *parent)
    : QWidget(parent)
    , m_frame(new QFrame(this))
    , m_vlayout(new QVBoxLayout(this))
    , m_flayout(new QVBoxLayout)
    , m_layout(new QGridLayout)
    , m_label(new QLabel(name, this))
{
	m_flayout->addWidget(m_label);
	m_flayout->addLayout(m_layout);

	m_frame->setLayout(m_flayout);
	m_frame->setFrameStyle(QFrame::StyledPanel);
	m_frame->setLineWidth(1);
	m_frame->setAutoFillBackground(true);

	m_vlayout->addWidget(m_frame);
	setLayout(m_vlayout);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

ModifierItem::~ModifierItem()
{
	delete m_flayout;
}

QGridLayout *&ModifierItem::layout()
{
	return m_layout;
}

void ModifierItem::setModifierData(Modifier *modifier)
{
	m_modifier_data = modifier;
}

Modifier *ModifierItem::modifierData() const
{
	return m_modifier_data;
}
