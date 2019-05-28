#pragma once

#include "core_types.h"
#include "hal/platform_memory.h"
#include "templates/enable_if.h"
#include "templates/is_trivial.h"

/**
 * @struct IsTriviallyCopyable templates/is_trivially_copyable.h
 * 
 * Sets value to true if copy constructor is trivial or deleted
 */
template<typename T>
struct IsTriviallyCopyable { enum {value = __has_trivial_copy(T)}; };

/// Move or copy buffer
/// @{
template<typename T>
FORCE_INLINE typename EnableIf<IsTriviallyCopyable<T>::value, void>::Type copy(T * dest, const T * src, uint64 n)
{
	// Copy memory
	PlatformMemory::memcpy(dest, src, n * sizeof(T));
}
template<typename T>
FORCE_INLINE typename EnableIf<!IsTriviallyCopyable<T>::value, void>::Type copy(T * dest, const T * src, int64 n)
{
	// Copy construct each element
	int64 i = 0;

	for (; i < n - 8; i += 8)
	{
			  T * _dest	= dest + i;
		const T * _src	= src + i;

		_dest[0] = _src[0];
		_dest[1] = _src[1];
		_dest[2] = _src[2];
		_dest[3] = _src[3];
		_dest[4] = _src[4];
		_dest[5] = _src[5];
		_dest[6] = _src[6];
		_dest[7] = _src[7];
	}
	for (; i < n - 4; i += 4)
	{
			  T * _dest	= dest + i;
		const T * _src	= src + i;
		
		_dest[0] = _src[0];
		_dest[1] = _src[1];
		_dest[2] = _src[2];
		_dest[3] = _src[3];
	}
	for (; i < n; ++i)
		dest[i] = src[i];
}
/// @}

/**
 * Swap two values
 * 
 * @param [in] a,b values to swap
 * @{
 */
template<typename T>
FORCE_INLINE typename EnableIf<IsTriviallyCopyable<T>::value, void>::Type swap(T & a, T & b)
{
	T t = a;
	a = b;
	b = t;
}
template<typename T>
FORCE_INLINE typename EnableIf<!IsTriviallyCopyable<T>::value, void>::Type swap(T & a, T & b)
{
	T t(a);
	new (&a) T(b);
	new (&b) T(t);
}
/// @}
