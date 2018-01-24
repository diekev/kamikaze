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

#include "custom_widgets.h"

class QGridLayout;

/* ********************************** */

/* Params are subclasses of their equivalent QWidgets. To avoid weird APIs, they
 * store a pointer to the external value they are 'connected' to. */

class FloatParam final : public FloatSpinBox {
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

class IntParam final : public IntSpinBox {
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

	std::string *m_value_ptr;

public:
	explicit StringParam(QWidget *parent = nullptr);
	~StringParam() = default;

	void valuePtr(std::string *ptr);

private Q_SLOTS:
	void updateValuePtr();

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

class FileParam final : public FileSelector {
	Q_OBJECT

	std::string *m_value_ptr;

public:
	explicit FileParam(bool input, QWidget *parent = nullptr);
	~FileParam() = default;

	void valuePtr(std::string *ptr);

private Q_SLOTS:
	void updateValuePtr(const QString &value);

Q_SIGNALS:
	void paramChanged();
};

/* ********************************** */

class ListParam final : public ListSelector {
	Q_OBJECT

	std::string *m_value_ptr;

public:
	explicit ListParam(QWidget *parent = nullptr);

	~ListParam() = default;

	void valuePtr(std::string *ptr);

private Q_SLOTS:
	void updateValuePtr(const QString &value);

Q_SIGNALS:
	void paramChanged();
};
