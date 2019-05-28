#pragma once

#include "core_types.h"

/**
 * @struct IsPointer templates/is_pointer.h
 * 
 * Sets value to true if type is a pointer
 * @{
 */
template<typename T>
struct IsPointer { enum {value = false}; };

template<typename T> struct IsPointer<T*> { enum {value = true}; };
/// @}

/**
 * @struct RemovePointer templates/pointer.h
 * 
 * Sets type to pointer underlying type
 * @{
 */
template<typename T>
struct RemovePointer {};

template<typename T> struct RemovePointer<T*> { using Type = T; };
/// @}

/**
 * @struct RemovePointers templates/pointer.h
 * 
 * Removes all pointers from type
 * @{
 */
template<typename T>
struct RemovePointers { using Type = T; };

template<typename T> struct RemovePointers<T*> { using Type = typename RemovePointers<T>::Type; };
/// @}