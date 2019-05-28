#pragma once

/**
 * @struct IsSame templates/is_same_type.h
 * 
 * Sets value to true if T and U
 * are the same type
 */
template<typename T, typename U>
struct IsSameType { enum {value = false}; };

template<typename T> struct IsSameType<T, T> { enum {value = true}; };
/// @}