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

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QWidget>

#include "xyzspinbox.h"

class QGridLayout;

/* ********************************** */

/* Params are subclasses of their equivalent QWidgets. To avoid weird APIs, they
 * store a pointer to the external value they are 'connected' to. */

class FloatParam final : public QDoubleSpinBox {
	Q_OBJECT

	float *m_value_ptr;

public:
	explicit FloatParam(QWidget *parent = nullptr);
	~FloatParam() = default;

	void valuePtr(float *ptr);

private Q_SLOTS:
	void updateValuePtr(double value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class IntParam final : public QSpinBox {
	Q_OBJECT

	int *m_value_ptr;

public:
	explicit IntParam(QWidget *parent = nullptr);
	~IntParam() = default;

	void valuePtr(int *ptr);

private Q_SLOTS:
	void updateValuePtr(int value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class BoolParam final : public QCheckBox {
	Q_OBJECT

	bool *m_value_ptr;

public:
	explicit BoolParam(QWidget *parent = nullptr);
	~BoolParam() = default;

	void valuePtr(bool *ptr);

private Q_SLOTS:
	void updateValuePtr(bool value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class EnumParam final : public QComboBox {
	Q_OBJECT

	int *m_value_ptr;

public:
	explicit EnumParam(QWidget *parent = nullptr);
	~EnumParam() = default;

	void valuePtr(int *ptr);

private Q_SLOTS:
	void updateValuePtr(int value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class StringParam final : public QLineEdit {
	Q_OBJECT

	QString *m_value_ptr;

public:
	explicit StringParam(QWidget *parent = nullptr);
	~StringParam() = default;

	void valuePtr(QString *ptr);

private Q_SLOTS:
	void updateValuePtr(const QString &value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class XYZParam final : public XYZSpinBox {
	Q_OBJECT

	float *m_value_ptr;

public:
	explicit XYZParam(QWidget *parent = nullptr);
	~XYZParam() = default;

	void valuePtr(float ptr[3]);

private Q_SLOTS:
	void updateValuePtr(double value, int axis);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class ParamCallback {
	QGridLayout *m_layout;
	QWidget *m_last_widget;
	int m_item_count;

	std::vector<QWidget *> m_widgets;

public:
	explicit ParamCallback(QGridLayout *layout);

	void addWidget(QWidget *widget, const QString &name);
	void setTooltip(const QString &tooltip);

	template <typename SlotType>
	void setContext(QObject *context, SlotType slot)
	{
		for (auto &widget : m_widgets) {
			QObject::connect(widget, SIGNAL(paramChanged()), context, slot);
		}
	}
};

/* ********************************** */

void int_param(ParamCallback &cb, const char *name, int *ptr, int min, int max, int default_value);
void float_param(ParamCallback &cb, const char *name, float *ptr, float min, float max, float default_value);
void enum_param(ParamCallback &cb, const char *name, int *ptr, const char *items[], int default_value);
void string_param(ParamCallback &cb, const char *name, QString *ptr, const char *default_value);
void bool_param(ParamCallback &cb, const char *name, bool *ptr, bool default_value);
void xyz_param(ParamCallback &cb, const char *name, float ptr[]);

void param_tooltip(ParamCallback &cb, const char *tooltip);
