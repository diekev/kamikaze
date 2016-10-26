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

#include "paramcallback.h"

#include <QGridLayout>
#include <QLabel>

#include "utils_ui.h"

ParamCallback::ParamCallback(QGridLayout *layout)
    : m_layout(layout)
    , m_last_widget(nullptr)
    , m_item_count(0)
{}

ParamCallback::~ParamCallback()
{
	clear();
}

void ParamCallback::addWidget(QWidget *widget, const QString &name)
{
	auto label = new QLabel(name);
	m_layout->addWidget(label, m_item_count, 0);
	m_layout->addWidget(widget, m_item_count, 1);

	m_last_widget = widget;
	m_widgets.push_back(widget);
	m_widget_map[name.toStdString()] = std::make_pair(label, widget);

	++m_item_count;
}

void ParamCallback::setTooltip(const QString &tooltip)
{
	if (m_last_widget) {
		m_last_widget->setToolTip(tooltip);
	}
}

void ParamCallback::clear()
{
	m_widget_map.clear();
	m_widgets.clear();
	clear_layout(m_layout);
}

void ParamCallback::setVisible(bool yesno)
{
	if (m_last_widget) {
		m_last_widget->setVisible(yesno);
	}
}

void ParamCallback::setVisible(const QString &name, bool yesno)
{
	widget_pair widgets = m_widget_map[name.toStdString()];
	widgets.first->setVisible(yesno);
	widgets.second->setVisible(yesno);
}

void ParamCallback::addWarning(const QString &warning)
{
	auto label = new QLabel("Warning");
	label->setPixmap(QPixmap("icons/warning.png"));

	m_layout->addWidget(label, m_item_count, 0, Qt::AlignRight);
	m_layout->addWidget(new QLabel(warning), m_item_count, 1);

	++m_item_count;
}
