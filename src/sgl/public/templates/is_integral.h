#pragma once

#include "core_types.h"

/**
 * @struct IsIntegral templates/is_integral.h
 * 
 * Sets value to true if type is integral
 * @{
 */
template<typename T>
struct IsIntegral { enum {value = false}; };

template<> struct IsIntegral<int8>	{ enum {value = true}; };
template<> struct IsIntegral<int16>	{ enum {value = true}; };
template<> struct IsIntegral<int32>	{ enum {value = true}; };
template<> struct IsIntegral<int64>	{ enum {value = true}; };

template<> struct IsIntegral<uint8>		{ enum {value = true}; };
template<> struct IsIntegral<uint16>	{ enum {value = true}; };
template<> struct IsIntegral<uint32>	{ enum {value = true}; };
template<> struct IsIntegral<uint64>	{ enum {value = true}; };

template<> struct IsIntegral<bool> { enum {value = true}; };

template<> struct IsIntegral<ansichar>	{ enum {value = true}; };
template<> struct IsIntegral<widechar>	{ enum {value = true}; };
/// @}

/**
 * @struct RemoveSign templates/unsigned.h
 * 
 * Return unsigned version of integer
 * @{
 */
template<typename T>
struct RemoveSign { using Type = T; };

template<> struct RemoveSign<uint8>		{ using Type = int8; };
template<> struct RemoveSign<uint16>	{ using Type = int16; };
template<> struct RemoveSign<uint32>	{ using Type = int32; };
template<> struct RemoveSign<uint64>	{ using Type = int64; };
/// @}

/**
 * @struct AddSign templates/unsigned.h
 * 
 * Return signed version of integer
 * @{
 */
template<typename T>
struct AddSign { using Type = T; };

template<> struct AddSign<int8>		{ using Type = uint8; };
template<> struct AddSign<int16>	{ using Type = uint16; };
template<> struct AddSign<int32>	{ using Type = uint32; };
template<> struct AddSign<int64>	{ using Type = uint64; };
/// @}