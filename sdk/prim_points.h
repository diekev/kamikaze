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

class PrimPoints : public Primitive {
	PointList m_points;
	std::vector<Attribute *> m_attributes;

	ego::BufferObject::Ptr m_buffer_data;
	ego::Program m_program;
	size_t m_elements;

public:
	PrimPoints();
	~PrimPoints();

	PointList *points();

	const PointList *points() const;

	Attribute *attribute(const std::string &name, AttributeType type);

	void addAttribute(Attribute *attr);

	Attribute *addAttribute(const std::string &name, AttributeType type, size_t size);

	Primitive *copy() const override;

	void render(ViewerContext *context, const bool for_outline) override;

	void setCustomUIParams(ParamCallback *cb) override;

	void prepareRenderData() override;

	void computeBBox(glm::vec3 &min, glm::vec3 &max) override;

	void loadShader();

	static size_t id;
	size_t typeID() const override;
};
