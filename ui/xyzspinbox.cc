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

#include "xyzspinbox.h"

#include <QDoubleSpinBox>
#include <QVBoxLayout>

enum {
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,
};

XYZSpinBox::XYZSpinBox(QWidget *parent)
    : QWidget(parent)
    , m_x(new QDoubleSpinBox(this))
    , m_y(new QDoubleSpinBox(this))
    , m_z(new QDoubleSpinBox(this))
    , m_layout(new QVBoxLayout(this))
{
	connect(m_x, SIGNAL(valueChanged(double)), this, SLOT(xValueChanged(double)));
	connect(m_y, SIGNAL(valueChanged(double)), this, SLOT(yValueChanged(double)));
	connect(m_z, SIGNAL(valueChanged(double)), this, SLOT(zValueChanged(double)));

	m_x->setAlignment(Qt::AlignRight);
	m_y->setAlignment(Qt::AlignRight);
	m_z->setAlignment(Qt::AlignRight);

	m_x->setSingleStep(0.01);
	m_y->setSingleStep(0.01);
	m_z->setSingleStep(0.01);

	m_layout->addWidget(m_x);
	m_layout->addWidget(m_y);
	m_layout->addWidget(m_z);

	m_layout->setSpacing(0);
}

void XYZSpinBox::xValueChanged(double value)
{
	Q_EMIT valueChanged(value, AXIS_X);
}

void XYZSpinBox::yValueChanged(double value)
{
	Q_EMIT valueChanged(value, AXIS_Y);
}

void XYZSpinBox::zValueChanged(double value)
{
	Q_EMIT valueChanged(value, AXIS_Z);
}

void XYZSpinBox::setValue(float *value)
{
	m_x->setValue(value[AXIS_X]);
	m_y->setValue(value[AXIS_Y]);
	m_z->setValue(value[AXIS_Z]);
}

void XYZSpinBox::getValue(float *value) const
{
	value[AXIS_X] = m_x->value();
	value[AXIS_Y] = m_y->value();
	value[AXIS_Z] = m_z->value();
}

void XYZSpinBox::setMinMax(float min, float max) const
{
	m_x->setMinimum(min);
	m_y->setMinimum(min);
	m_z->setMinimum(min);

	m_x->setMaximum(max);
	m_y->setMaximum(max);
	m_z->setMaximum(max);
}
