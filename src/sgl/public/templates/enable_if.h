#pragma once

#include "core_types.h"

/**
 * @struct EnableIf templates/enable_if.h
 * 
 * Sets Type to specified type if boolean condition is true
 * @{
 */
template<bool Cond, typename T = void>
struct EnableIf {};

template<typename T> struct EnableIf<true, T> { using Type = T; };
/// @}

