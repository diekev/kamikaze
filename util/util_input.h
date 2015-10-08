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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2015 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#pragma once

enum {
	MOUSE_NONE         = -1,
	MOUSE_LEFT         = 0,
	MOUSE_MIDDLE       = 1,
	MOUSE_RIGHT        = 2,
	MOUSSE_SCROLL_UP   = 3,
	MOUSSE_SCROLL_DOWN = 4,
};

enum {
	MOD_KEY_NONE  = 0,
	MOD_KEY_SHIFT = 1,
	MOD_KEY_CTRL  = 2,
	MOD_KEY_ALT   = 3,
};

enum {
	MOUSE_DOWN = 0,
	MOUSE_UP   = 1,
};
