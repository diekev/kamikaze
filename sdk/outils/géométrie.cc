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
 * The Original Code is Copyright (C) 2017 Kévin Dietrich.
 * All rights reserved.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

#include "géométrie.h"

#include "../attribute.h"
#include "../geomlists.h"

#include "parallélisme.h"

void calcule_normales(
		const PointList &points,
		const PolygonList &polygones,
		Attribute &normales,
		bool flip)
{
	parallel_for(tbb::blocked_range<size_t>(0, polygones.size()),
				 [&](const tbb::blocked_range<size_t> &r)
	{
		for (auto i = r.begin(), ie = r.end(); i < ie ; ++i) {
			const auto &quad = polygones[i];

			const auto v0 = points[quad[0]];
			const auto v1 = points[quad[1]];
			const auto v2 = points[quad[2]];

			const auto normal = normale_triangle(v0, v1, v2);

			normales.vec3(quad[0], normales.vec3(quad[0]) + normal);
			normales.vec3(quad[1], normales.vec3(quad[1]) + normal);
			normales.vec3(quad[2], normales.vec3(quad[2]) + normal);

			if (quad[3] != INVALID_INDEX) {
				normales.vec3(quad[3], normales.vec3(quad[3]) + normal);
			}
		}
	});

	if (flip) {
		for (size_t i = 0, ie = points.size(); i < ie ; ++i) {
			normales.vec3(i, glm::normalize(normales.vec3(i)));
		}
	}
	else {
		for (size_t i = 0, ie = points.size(); i < ie ; ++i) {
			normales.vec3(i, -glm::normalize(normales.vec3(i)));
		}
	}
}
