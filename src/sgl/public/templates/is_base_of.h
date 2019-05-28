#pragma once

#include "core_types.h"
#include <type_traits>

/**
 * @struct IsBaseOf templates/is_base_of.h
 * 
 * Sets value to true if target derives from base class
 */
template<typename BaseT, typename DerivedT>
struct IsBaseOf { enum {value = __is_base_of(BaseT, DerivedT)}; };

