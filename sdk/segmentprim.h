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

#include "attribute.h"
#include "geomlists.h"
#include "primitive.h"

class RenderBuffer;

class SegmentPrim : public Primitive {
	PointList m_points;
	EdgeList m_edges;

	RenderBuffer *m_renderbuffer;

	size_t m_nombre_courbes = 0;
	size_t m_points_par_courbe = 0;

public:
	SegmentPrim();
	SegmentPrim(const SegmentPrim &other);
	~SegmentPrim();

	PointList *points();

	const PointList *points() const;

	EdgeList *edges();

	const EdgeList *edges() const;

	size_t nombre_courbes() const;

	void nombre_courbes(size_t nombre);

	size_t points_par_courbe() const;

	void points_par_courbe(size_t nombre);

	Primitive *copy() const override;

	void render(const ViewerContext &context) override;

	void prepareRenderData() override;

	void computeBBox(glm::vec3 &min, glm::vec3 &max) override;

	void loadShader();

	static size_t id;
	size_t typeID() const override;
};
