#pragma once

#include "core_types.h"

/**
 * @struct RemoveReference templates/reference.h
 * 
 * Struct to strip the reference from a data type
 * @{
 */
template<typename T> struct RemoveReference			{ using Type = T; };
template<typename T> struct RemoveReference<T&>		{ using Type = T; };
template<typename T> struct RemoveReference<T&&>	{ using Type = T; };
/// @}

/**
 * @struct IsReference templates/reference.h
 * 
 * Sets value to true if T is a reference of some type
 * @{
 */
template<typename T>
struct IsReference { enum {value = false}; };

template<typename T> struct IsReference<T&>			{ enum {value = true}; };
template<typename T> struct IsReference<const T&>	{ enum {value = true}; };
template<typename T> struct IsReference<T&&>		{ enum {value = true}; };
/// @}

/**
 * @struct IsReferenceOf templates/reference.h
 * 
 * Sets value to true if T is a reference of type U
 * @{
 */

template<typename T, typename U>
struct IsReferenceOf { enum {value = false}; };

template<typename U> struct IsReferenceOf<U&, U>		{ enum {value = true}; };
template<typename U> struct IsReferenceOf<const U&, U>	{ enum {value = true}; };
template<typename U> struct IsReferenceOf<U&&, U>		{ enum {value = true}; };
/// @}

/**
 * @struct ConstRef templates/reference.h
 * 
 * Sets Type to a constant reference if
 * sizeof type is greater than sizeof pointer
 * @{
 */
template<typename T, bool = (sizeof(T) > sizeof(void*))>
struct ConstRef {};

template<typename T> struct ConstRef<T, false>	{ using Type = T; };
template<typename T> struct ConstRef<T, true>	{ using Type = const T&; };
/// @}

/// @brief Casts a reference to an rvalue reference
template<typename T>
FORCE_INLINE CONSTEXPR typename RemoveReference<T>::Type && move(T && obj)
{
	return static_cast<typename RemoveReference<T>::Type&&>(obj);
}

/// Forwards lvalues as lvalues or as rvalues
/// @{
template<typename T>
FORCE_INLINE CONSTEXPR T && forward(typename RemoveReference<T>::Type & obj)
{
	return static_cast<T&&>(obj);
}
template<typename T>
FORCE_INLINE CONSTEXPR T && forward(typename RemoveReference<T>::Type && obj)
{
	return static_cast<T&&>(obj);
}
/// @}
