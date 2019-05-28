#pragma once

#include "core_types.h"
#include "reference.h"

/**
 * @class Compare templates/functional.h
 * 
 * Three-way comparison operator
 */
template<typename A, typename B = A>
struct Compare
{
	/**
	 * Compares two operands.
	 * Requires operator< and operator> to be defined for both types
	 * 
	 * @param [in] a,b operands to compare
	 * @return -1, 0 or 1
	 */
	FORCE_INLINE int32 operator()(typename ConstRef<A>::Type a, typename ConstRef<B>::Type b) const
	{
		return int32(a > b) - int32(a < b);
	}
};