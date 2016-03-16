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

#include "paramfactory.h"

#include <QLabel>
#include <QGridLayout>

/* ********************************** */

FloatParam::FloatParam(QWidget *parent)
    : QDoubleSpinBox(parent)
    , m_value_ptr(nullptr)
{
	setValue(0.0);
	setAlignment(Qt::AlignRight);
	connect(this, SIGNAL(valueChanged(double)), this, SLOT(updateValuePtr(double)));
}

void FloatParam::valuePtr(float *ptr)
{
	m_value_ptr = ptr;
	setValue(*ptr);
}

void FloatParam::updateValuePtr(double value)
{
	*m_value_ptr = static_cast<float>(value);
}

/* ********************************** */

IntParam::IntParam(QWidget *parent)
    : QSpinBox(parent)
    , m_value_ptr(nullptr)
{
	setValue(0);
	setAlignment(Qt::AlignRight);
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValuePtr(int)));
}

void IntParam::valuePtr(int *ptr)
{
	m_value_ptr = ptr;
	setValue(*ptr);
}

void IntParam::updateValuePtr(int value)
{
	*m_value_ptr = value;
}

/* ********************************** */

BoolParam::BoolParam(QWidget *parent)
    : QCheckBox(parent)
    , m_value_ptr(nullptr)
{
	setChecked(false);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(updateValuePtr(bool)));
}

void BoolParam::valuePtr(bool *ptr)
{
	m_value_ptr = ptr;
	setChecked(*ptr);
}

void BoolParam::updateValuePtr(bool value)
{
	*m_value_ptr = value;
}

/* ********************************** */

EnumParam::EnumParam(QWidget *parent)
    : QComboBox(parent)
    , m_value_ptr(nullptr)
{
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValuePtr(int)));
}

void EnumParam::valuePtr(int *ptr)
{
	m_value_ptr = ptr;
}

void EnumParam::updateValuePtr(int value)
{
	*m_value_ptr = value;
}

/* ********************************** */

StringParam::StringParam(QWidget *parent)
    : QLineEdit(parent)
    , m_value_ptr(nullptr)
{
	connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateValuePtr(const QString &)));
}

void StringParam::valuePtr(QString *ptr)
{
	m_value_ptr = ptr;
}

void StringParam::updateValuePtr(const QString &value)
{
	*m_value_ptr = value;
}

/* ********************************** */

XYZParam::XYZParam(QWidget *parent)
    : XYZSpinBox(parent)
    , m_value_ptr(nullptr)
{
	connect(this, SIGNAL(valueChanged(double, int)), this, SLOT(updateValuePtr(double, int)));
}

void XYZParam::valuePtr(float ptr[3])
{
	m_value_ptr = ptr;
	setValue(ptr);
}

void XYZParam::updateValuePtr(double value, int axis)
{
	m_value_ptr[axis] = static_cast<float>(value);
}

/* ********************************** */

ParamCallback::ParamCallback(QGridLayout *layout)
    : m_layout(layout)
    , m_last_widget(nullptr)
    , m_item_count(0)
{}

void ParamCallback::addWidget(QWidget *widget, const QString &name)
{
	m_layout->addWidget(new QLabel(name), m_item_count, 0);
	m_layout->addWidget(widget, m_item_count, 1);

	m_last_widget = widget;

	++m_item_count;
}

void ParamCallback::setTooltip(const QString &tooltip)
{
	if (m_last_widget) {
		m_last_widget->setToolTip(tooltip);
	}
}

/* ********************************** */

void int_param(ParamCallback &cb, const char *name, int *ptr, int min, int max, int default_value)
{
	auto param = new IntParam;
	param->valuePtr(ptr);
	param->setRange(min, max);
	param->setValue(default_value);

	cb.addWidget(param, name);
}

void float_param(ParamCallback &cb, const char *name, float *ptr, float min, float max, float default_value)
{
	auto param = new FloatParam;
	param->valuePtr(ptr);
	param->setRange(min, max);
	param->setValue(default_value);

	cb.addWidget(param, name);
}

void enum_param(ParamCallback &cb, const char *name, int *ptr, const char *items[], int default_value)
{
	auto param = new EnumParam;
	param->valuePtr(ptr);

	while (*items != nullptr) {
		param->addItem(*items++);
	}

	param->setCurrentIndex(default_value);

	cb.addWidget(param, name);
}

void string_param(ParamCallback &cb, const char *name, QString *ptr, const char *default_value)
{
	auto param = new StringParam;
	param->valuePtr(ptr);
	param->setPlaceholderText(default_value);

	cb.addWidget(param, name);
}

void bool_param(ParamCallback &cb, const char *name, bool *ptr, bool default_value)
{
	auto param = new BoolParam;
	param->valuePtr(ptr);
	param->setChecked(default_value);

	cb.addWidget(param, name);
}

void param_tooltip(ParamCallback &cb, const char *tooltip)
{
	cb.setTooltip(tooltip);
}

void xyz_param(ParamCallback &cb, const char *name, float ptr[3])
{
	auto param = new XYZParam;
	param->valuePtr(ptr);

	cb.addWidget(param, name);
}
