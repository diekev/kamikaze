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

#include "custom_widgets.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

enum {
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,
};

/* ********************************** */

FloatSpinBox::FloatSpinBox(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
    , m_spin_box(new QDoubleSpinBox(this))
    , m_slider(new QSlider(Qt::Orientation::Horizontal, this))
    , m_scale(1.0f)
{
	m_layout->addWidget(m_spin_box);
	m_layout->addWidget(m_slider);

	setLayout(m_layout);

	m_spin_box->setAlignment(Qt::AlignRight);
	m_spin_box->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_spin_box->setReadOnly(true);

	connect(m_slider, SIGNAL(sliderReleased()), this, SLOT(ValueChanged()));
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(updateLabel(int)));
}

void FloatSpinBox::ValueChanged()
{
	const auto value = m_slider->value();
	const float fvalue = value / m_scale;
	m_spin_box->setValue(fvalue);
	Q_EMIT(valueChanged(fvalue));
}

void FloatSpinBox::updateLabel(int value)
{
	m_spin_box->setValue(value / m_scale);
}

void FloatSpinBox::setValue(float value)
{
	m_spin_box->setValue(value);
	m_slider->setValue(value * m_scale);
}

float FloatSpinBox::value() const
{
	return m_spin_box->value();
}

void FloatSpinBox::setRange(float min, float max)
{
	if (min != 0.0f && min < 1.0f) {
		m_scale = 1.0f / min;
	}
	else {
		m_scale = 10000.0f;
	}

	m_slider->setRange(min * m_scale, max * m_scale);
}

/* ********************************** */

IntSpinBox::IntSpinBox(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
    , m_spin_box(new QSpinBox(this))
    , m_slider(new QSlider(Qt::Orientation::Horizontal, this))
{
	m_layout->addWidget(m_spin_box);
	m_layout->addWidget(m_slider);

	setLayout(m_layout);

	m_spin_box->setAlignment(Qt::AlignRight);
	m_spin_box->setButtonSymbols(QAbstractSpinBox::NoButtons);
	m_spin_box->setReadOnly(true);

	connect(m_slider, SIGNAL(sliderReleased()), this, SLOT(ValueChanged()));
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(updateLabel(int)));
}

void IntSpinBox::ValueChanged()
{
	const auto value = m_slider->value();
	m_spin_box->setValue(value);
	Q_EMIT(valueChanged(value));
}

void IntSpinBox::updateLabel(int value)
{
	m_spin_box->setValue(value);
}

void IntSpinBox::setValue(int value)
{
	m_spin_box->setValue(value);
	m_slider->setValue(value);
}

int IntSpinBox::value() const
{
	return m_spin_box->value();
}

void IntSpinBox::setRange(int min, int max)
{
	m_slider->setRange(min, max);
}

/* ********************************** */

XYZSpinBox::XYZSpinBox(QWidget *parent)
    : QWidget(parent)
    , m_x(new FloatSpinBox(this))
    , m_y(new FloatSpinBox(this))
    , m_z(new FloatSpinBox(this))
    , m_layout(new QVBoxLayout(this))
{
	connect(m_x, SIGNAL(valueChanged(double)), this, SLOT(xValueChanged(double)));
	connect(m_y, SIGNAL(valueChanged(double)), this, SLOT(yValueChanged(double)));
	connect(m_z, SIGNAL(valueChanged(double)), this, SLOT(zValueChanged(double)));

	m_layout->addWidget(m_x);
	m_layout->addWidget(m_y);
	m_layout->addWidget(m_z);

	m_layout->setSpacing(0);
}

void XYZSpinBox::xValueChanged(double value)
{
	Q_EMIT(valueChanged(value, AXIS_X));
}

void XYZSpinBox::yValueChanged(double value)
{
	Q_EMIT(valueChanged(value, AXIS_Y));
}

void XYZSpinBox::zValueChanged(double value)
{
	Q_EMIT(valueChanged(value, AXIS_Z));
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
	m_x->setRange(min, max);
	m_y->setRange(min, max);
	m_z->setRange(min, max);
}

/* ********************************** */

FileSelector::FileSelector(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QHBoxLayout(this))
    , m_line_edit(new QLineEdit(this))
    , m_push_button(new QPushButton("Open File", this))
{
	m_layout->addWidget(m_line_edit);
	m_layout->addWidget(m_push_button);

	setLayout(m_layout);

	connect(m_push_button, SIGNAL(clicked()), this, SLOT(setChoosenFile()));
}

void FileSelector::setValue(const QString &text)
{
	m_line_edit->setText(text);
}

void FileSelector::setChoosenFile()
{
	const auto filename = QFileDialog::getSaveFileName(this);

	if (!filename.isEmpty()) {
		m_line_edit->setText(filename);
		Q_EMIT(textChanged(filename));
	}
}
