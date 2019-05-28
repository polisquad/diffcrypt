#pragma once

#include "core_types.h"

/**
 * @struct IsTrivial templates/is_trivial.h
 * 
 * Sets value to true if type is trivial
 */
template<typename T>
struct IsTrivial { enum {value = __is_trivial(T)}; };
