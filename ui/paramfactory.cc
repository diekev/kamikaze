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

#include <kamikaze/persona.h>

#include "intern/param_widgets.h"

#include "paramcallback.h"

/* ********************************** */

void int_param(ParamCallback *cb, const char *name, int *ptr, int min, int max, int default_value)
{
	auto param = new IntParam;
	param->valuePtr(ptr);
	param->setRange(min, max);
	param->setValue(default_value);

	cb->addWidget(param, name);
}

void float_param(ParamCallback *cb, const char *name, float *ptr, float min, float max, float default_value)
{
	auto param = new FloatParam;
	param->valuePtr(ptr);
	param->setRange(min, max);
	param->setValue(default_value);

	cb->addWidget(param, name);
}

void enum_param(ParamCallback *cb, const char *name, int *ptr, const EnumProperty &prop, int default_value)
{
	auto param = new EnumParam;
	param->valuePtr(ptr);

	for (const auto &item : prop.m_props) {
		param->addItem(item.name.c_str());
	}

	param->setCurrentIndex(default_value);

	cb->addWidget(param, name);
}

void string_param(ParamCallback *cb, const char *name, std::string *ptr, const char *default_value)
{
	auto param = new StringParam;
	param->valuePtr(ptr);

	if (ptr->length() == 0) {
		param->setPlaceholderText(default_value);
	}
	else {
		param->setText(ptr->c_str());
	}

	cb->addWidget(param, name);
}

void bool_param(ParamCallback *cb, const char *name, bool *ptr, bool default_value)
{
	auto param = new BoolParam;
	param->valuePtr(ptr);
	param->setChecked(default_value);

	cb->addWidget(param, name);
}

void param_tooltip(ParamCallback *cb, const char *tooltip)
{
	cb->setTooltip(tooltip);
}

void xyz_param(ParamCallback *cb, const char *name, float ptr[3], float min, float max)
{
	auto param = new XYZParam;
	param->setMinMax(min, max);
	param->valuePtr(ptr);

	cb->addWidget(param, name);
}

void input_file_param(ParamCallback *cb, const char *name, std::string *ptr)
{
	auto param = new FileParam(true);
	param->valuePtr(ptr);
	param->setValue(ptr->c_str());

	cb->addWidget(param, name);
}

void output_file_param(ParamCallback *cb, const char *name, std::string *ptr)
{
	auto param = new FileParam(false);
	param->valuePtr(ptr);
	param->setValue(ptr->c_str());

	cb->addWidget(param, name);
}
