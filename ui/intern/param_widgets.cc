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

#include "param_widgets.h"

/* ********************************** */

FloatParam::FloatParam(QWidget *parent)
    : FloatSpinBox(parent)
    , m_value_ptr(nullptr)
{
	setValue(0.0);
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
	Q_EMIT(paramChanged());
}

/* ********************************** */

IntParam::IntParam(QWidget *parent)
    : IntSpinBox(parent)
    , m_value_ptr(nullptr)
{
	setValue(0);
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
	Q_EMIT(paramChanged());
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
	Q_EMIT(paramChanged());
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
	Q_EMIT(paramChanged());
}

/* ********************************** */

StringParam::StringParam(QWidget *parent)
    : QLineEdit(parent)
    , m_value_ptr(nullptr)
{
	connect(this, SIGNAL(returnPressed()), this, SLOT(updateValuePtr()));
}

void StringParam::valuePtr(std::string *ptr)
{
	m_value_ptr = ptr;
}

void StringParam::updateValuePtr()
{
	*m_value_ptr = this->text().toStdString();
	Q_EMIT(paramChanged());
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
	Q_EMIT(paramChanged());
}

/* ********************************** */

FileParam::FileParam(bool input, QWidget *parent)
    : FileSelector(input, parent)
    , m_value_ptr(nullptr)
{
	connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateValuePtr(const QString &)));
}

void FileParam::valuePtr(std::string *ptr)
{
	m_value_ptr = ptr;
}

void FileParam::updateValuePtr(const QString &value)
{
	*m_value_ptr = value.toStdString();
	Q_EMIT(paramChanged());
}
