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

#include "brush.h"

/* Note: since we are dealing with level sets where exterior values are positive
 * and interior ones are negative, addition and subtraction are swapped.
 */

void addOp(float &value)
{
	value -= 0.1f;
}

void subOp(float &value)
{
	value += 0.1f;
}

Brush::Brush()
    : m_radius(0.0f)
    , m_inv_radius(0.0f)
    , m_amount(0.0f)
    , m_mode(BRUSH_MODE_ADD)
{}

Brush::Brush(const float radius, const float amount)
    : m_radius(radius)
    , m_inv_radius(1.0f / m_radius)
    , m_amount(amount)
    , m_mode(BRUSH_MODE_ADD)
{}

void Brush::radius(const float rad)
{
	m_radius = rad;
	m_inv_radius = 1.0f / m_radius;
}

float Brush::radius() const
{
	return m_radius;
}

void Brush::amount(const float amnt)
{
	m_amount = amnt;
}

float Brush::amount() const
{
	return ((m_mode == BRUSH_MODE_ADD) ? m_amount : -m_amount);
}

void Brush::mode(const int mode)
{
	m_mode = mode;
}
