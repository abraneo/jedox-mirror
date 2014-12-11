/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * 
 *
 */

#ifndef BIND_H
#define BIND_H

namespace Palo {
namespace Util {
/* DATA 2 FUNCTION BINDING */

template<template<typename func_ptr_t, func_ptr_t func_ptr, class base_type> class Data2FunctionBindingImpl,
typename func_ptr_t, class base_type>
class Data2FunctionBindingCreator;

template<typename func_ptr_t, func_ptr_t func_ptr, class base_type>
class Data2FunctionBindingImplDefault;

/*! \brief Maps data to a member function. */
template<template<typename func_ptr_t, func_ptr_t func_ptr, class base_type> class Data2FunctionBindingImpl = Data2FunctionBindingImplDefault>
class Data2FunctionBinding {
public:
	virtual ~Data2FunctionBinding()
	{
	}

	template<class base_type, typename func_ptr_t>
	const static Data2FunctionBindingCreator<Data2FunctionBindingImpl, func_ptr_t, base_type>& getCreator(func_ptr_t ptr)
	{
		static Data2FunctionBindingCreator<Data2FunctionBindingImpl, func_ptr_t, base_type> creator;
		return creator;
	}
};

/*! You have to implement something like this! */
template<typename func_ptr_t, func_ptr_t func_ptr, class base_type>
class Data2FunctionBindingImplDefault : public Data2FunctionBinding<> {
public:
	Data2FunctionBindingImplDefault()
	{
	}
	~Data2FunctionBindingImplDefault()
	{
	}

	virtual void PROVIDE_YOUR_OWN_TEPMLATE() = 0;
};

template<template<typename func_ptr_t, func_ptr_t func_ptr, class base_type> class Data2FunctionBindingImpl, typename func_ptr_t, class base_type>
class Data2FunctionBindingCreator {
public:
	template<func_ptr_t func_ptr>
	static Data2FunctionBindingImpl<func_ptr_t, func_ptr, base_type>& get()
	{
		static Data2FunctionBindingImpl<func_ptr_t, func_ptr, base_type> binding;
		return binding;
	}
};
}
}
#endif
