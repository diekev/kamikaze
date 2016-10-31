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

#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QVBoxLayout;

/* ********************************** */

class FloatSpinBox : public QWidget {
	Q_OBJECT

	QHBoxLayout *m_layout;
	QDoubleSpinBox *m_spin_box;
	QSlider *m_slider;

	float m_scale;

public:
	explicit FloatSpinBox(QWidget *parent = nullptr);
	~FloatSpinBox() = default;

	void setValue(float value);
	float value() const;
	void setRange(float min, float max);

Q_SIGNALS:
	void valueChanged(double value);

private Q_SLOTS:
	void ValueChanged();
	void updateLabel(int value);
};

/* ********************************** */

class IntSpinBox : public QWidget {
	Q_OBJECT

	QHBoxLayout *m_layout;
	QSpinBox *m_spin_box;
	QSlider *m_slider;

public:
	explicit IntSpinBox(QWidget *parent = nullptr);
	~IntSpinBox() = default;

	void setValue(int value);
	int value() const;
	void setRange(int min, int max);

Q_SIGNALS:
	void valueChanged(int value);

private Q_SLOTS:
	void ValueChanged();
	void updateLabel(int value);
};

/* ********************************** */

class XYZSpinBox : public QWidget {
	Q_OBJECT

	FloatSpinBox *m_x, *m_y, *m_z;
	QVBoxLayout *m_layout;

private Q_SLOTS:
	void xValueChanged(double value);
	void yValueChanged(double value);
	void zValueChanged(double value);

Q_SIGNALS:
	void valueChanged(double value, int axis);

public:
	explicit XYZSpinBox(QWidget *parent = nullptr);
	~XYZSpinBox() = default;

	void setValue(float *value);
	void getValue(float *value) const;
	void setMinMax(float min, float max) const;
};

/* ********************************** */

class FileSelector : public QWidget {
	Q_OBJECT

	QHBoxLayout *m_layout;
	QLineEdit *m_line_edit;
	QPushButton *m_push_button;

	bool m_input;

public:
	explicit FileSelector(bool input, QWidget *parent = nullptr);

	~FileSelector() = default;

	void setValue(const QString &text);

private Q_SLOTS:
	void setChoosenFile();

Q_SIGNALS:
	void textChanged(const QString &text);
};

/* ********************************** */

class QMenu;

class ListSelector : public QWidget {
	Q_OBJECT

	QHBoxLayout *m_layout;
	QLineEdit *m_line_edit;
	QPushButton *m_push_button;
	QMenu *m_list_widget;

	bool m_input;

public:
	explicit ListSelector(QWidget *parent = nullptr);

	~ListSelector();

	void setValue(const QString &text);

	void addField(const QString &text);

private Q_SLOTS:
	void showList();
	void handleClick();
	void updateText();

Q_SIGNALS:
	void textChanged(const QString &text);
};
