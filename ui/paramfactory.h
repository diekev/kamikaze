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

#include <string>

class EnumProperty;
class ParamCallback;
class QString;

/**
 * @brief int_param Add a UI parameter for an integer property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the integer property.
 * @param min  The minimum value the parameter can have.
 * @param max  The maximum value the parameter can have.
 * @param default_value The default value of the parameter.
 */
void int_param(ParamCallback &cb, const char *name, int *ptr, int min, int max, int default_value);

/**
 * @brief float_param Add a UI parameter for a float property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the float property.
 * @param min  The minimum value the parameter can have.
 * @param max  The maximum value the parameter can have.
 * @param default_value The default value of the parameter.
 */
void float_param(ParamCallback &cb, const char *name, float *ptr, float min, float max, float default_value);

/**
 * @brief enum_param Add a UI parameter for an enumeration property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the enumeration property.
 * @param items An array of names representing the possible enumeration values.
 *              The last item needs to be set to null.
 * @param default_value The default value of the parameter.
 */
void enum_param(ParamCallback &cb, const char *name, int *ptr, const EnumProperty &prop, int default_value);

/**
 * @brief string_param Add a UI parameter for a string property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the string property.
 * @param default_value Default string used as placeholder text.
 */
void string_param(ParamCallback &cb, const char *name, std::string *ptr, const char *default_value);

/**
 * @brief bool_param Add a UI parameter for a boolean property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the boolean property.
 * @param default_value The default value of the parameter.
 */
void bool_param(ParamCallback &cb, const char *name, bool *ptr, bool default_value);

/**
 * @brief bool_param Add a UI parameter for a vector property.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the vector property.
 */
void xyz_param(ParamCallback &cb, const char *name, float ptr[3], float min = 0.0f, float max = 10.0f);

/**
 * @brief file_param Add a UI parameter for displaying a file selector.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the file name property.
 */
void file_param(ParamCallback &cb, const char *name, std::string *ptr);

/**
 * @brief file_param Add a UI parameter for displaying a file selector.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the file name property.
 */
void input_file_param(ParamCallback &cb, const char *name, std::string *ptr);

/**
 * @brief file_param Add a UI parameter for displaying a file selector.
 *
 * @param cb   The callback used to create the parameter.
 * @param name The UI name of the parameter.
 * @param ptr  The pointer to the file name property.
 */
void output_file_param(ParamCallback &cb, const char *name, std::string *ptr);

/**
 * @brief param_tooltip Set the tooltip for the last added parameter.
 *
 * @param cb The callback used to create the parameter.
 * @param tooltip The parameters tooltip.
 */
void param_tooltip(ParamCallback &cb, const char *tooltip);
