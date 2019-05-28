#pragma once

#include "core_types.h"
#include "templates/reference.h"
#include "templates/functional.h"
#include "templates/is_same_type.h"

/**
 * @class Pair containers/pair.h
 * 
 * A pair of values
 */
template<typename A, typename B, typename CompareT = Compare<A>>
class Pair
{
public:
	/// First element
	A first;

	/// Second element
	B second;

public:
	/// Default constructor
	FORCE_INLINE Pair() = default;

	/// Pair constructor
	template<typename _A = A, typename _B = B>
	FORCE_INLINE Pair(_A && _first, _B && _second)
		: first{forward<A>(_first)}
		, second{forward<B>(_second)} {}
	
	/// Key copy constructor
	FORCE_INLINE Pair(const A & _first)
		: first{_first} {}
	
	/// Key move constructor
	FORCE_INLINE Pair(A && _first)
		: first{move(_first)} {}

	/// Equality operators
	/// @{
	FORCE_INLINE bool operator==(const Pair & other) const { return first == other.first & second == other.second; }
	FORCE_INLINE bool operator!=(const Pair & other) const { return first != other.first | second != other.second; }
	/// @}

	/// Key comparison operators
	/// @{
	FORCE_INLINE bool operator< (const Pair & other) const { return CompareT()(first, other.first) < 0; }
	FORCE_INLINE bool operator> (const Pair & other) const { return CompareT()(first, other.first) > 0; }
	FORCE_INLINE bool operator<=(const Pair & other) const { return CompareT()(first, other.first) <= 0; }
	FORCE_INLINE bool operator>=(const Pair & other) const { return CompareT()(first, other.first) >= 0; }
	/// @}

	/// Key comparison, pairs with different compare methods
	/// @{
	template<typename CompareU>
	FORCE_INLINE typename EnableIf<!IsSameType<CompareT, CompareU>::value, bool>::Type operator< (const Pair<A, B, CompareU> & other) { return CompareT()(first, other.first) < 0; }
	template<typename CompareU>
	FORCE_INLINE typename EnableIf<!IsSameType<CompareT, CompareU>::value, bool>::Type operator> (const Pair<A, B, CompareU> & other) { return CompareT()(first, other.first) > 0; }
	template<typename CompareU>
	FORCE_INLINE typename EnableIf<!IsSameType<CompareT, CompareU>::value, bool>::Type operator<=(const Pair<A, B, CompareU> & other) { return CompareT()(first, other.first) <= 0; }
	template<typename CompareU>
	FORCE_INLINE typename EnableIf<!IsSameType<CompareT, CompareU>::value, bool>::Type operator>=(const Pair<A, B, CompareU> & other) { return CompareT()(first, other.first) >= 0; }
	/// @}
};

