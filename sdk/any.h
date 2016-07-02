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

#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <utility>

class bad_any_cast : public std::bad_cast {
public:
	virtual const char *what() const noexcept;
};

class any {
	struct dummy;
	dummy *dt = nullptr;

public:
	any() noexcept = default;

	any(const any &other);

	any(any &&other) noexcept;

	template <typename T>
	any(T &&value)
	{
		try {
			dt = new data<typename std::decay<T>::type>;
			auto da = static_cast<data<T> *>(dt);
			da->val = std::forward<T>(value);
		}
		catch (...) {
			throw;
		}

		assert(!this->empty());
	}

	~any();

	/* Modifiers. */

	any &operator=(const any &other);
	any &operator=(any &&other) noexcept;

	template <typename T>
	any &operator=(T &&value)
	{
		try {
			dt = new data<typename std::decay<T>::type>;
			auto da = static_cast<data<T> *>(dt);
			da->val = std::forward<T>(value);
		}
		catch (...) {
			throw;
		}

		assert(!this->empty());
	}

	void clear();

	void swap(any &other) noexcept;

	/* Observers. */

	bool empty() const noexcept;

	const std::type_info &type() const noexcept;

	template <typename T>
	T *as_ptr()
	{
		auto da = static_cast<data<T> *>(dt);
		return &da->val;
	}

	template <typename T>
	const T *as_ptr() const
	{
		auto da = static_cast<data<T> *>(dt);
		return &da->val;
	}

private:
	struct dummy {
		virtual ~dummy() = default;
		virtual dummy *copy() = 0;
		virtual const std::type_info &type() const noexcept = 0;
	};

	template <typename T>
	struct data : public dummy {
		T val;

		dummy *copy()
		{
			auto cpy = new data<T>();
			static_cast<data<T> *>(cpy)->val = this->val;
			return cpy;
		}

		const std::type_info &type() const noexcept override
		{
			return typeid(val);
		}
	};
};

void swap(any &lhs, any &rhs);

template <typename T>
T any_cast(const any &operand)
{
	if (typeid(T) != operand.type()) {
		throw bad_any_cast();
	}

	return *any_cast<std::add_const_t<std::remove_reference_t<T>>>(&operand);
}

template <typename T>
T any_cast(any &operand)
{
	if (typeid(T) != operand.type()) {
		throw bad_any_cast();
	}

	return *any_cast<std::remove_reference_t<T>>(&operand);
}

template <typename T>
T any_cast(any &&operand)
{
	if (typeid(T) != operand.type()) {
		throw bad_any_cast();
	}

	return *any_cast<std::remove_reference_t<T>>(&operand);
}

template <typename T>
const T *any_cast(const any *operand) noexcept
{
	using nptr_type = typename std::remove_pointer<T>::type;
	using type = typename std::remove_const<nptr_type>::type;

	if (operand->empty() || typeid(type) != operand->type()) {
		return nullptr;
	}

	return operand->as_ptr<type>();
}

template <typename T>
T *any_cast(any *operand) noexcept
{
	using nptr_type = typename std::remove_pointer<T>::type;
	using type = typename std::remove_const<nptr_type>::type;

	if (operand->empty() || typeid(type) != operand->type()) {
		return nullptr;
	}

	return operand->as_ptr<type>();
}
