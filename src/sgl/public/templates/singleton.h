#pragma once

#include "core_types.h"

/**
 * @class Singletong templates/singleton.h
 * Defines a singleton with methods to access it
 */
template<class T>
class Singleton
{
protected:
	/// Global singleton instance
	static T * instance;

public:
	/// Default constructor
	FORCE_INLINE Singleton()
	{
		instance = reinterpret_cast<T*>(this);
	}

	/// Deleted copy constructor
	Singleton(const Singleton & s) = delete;

	/// Deleted copy assignment
	Singleton & operator=(const Singleton & s) = delete;

	/**
	 * Get instance reference
	 * 
	 * @return Reference to instance
	 */
	static FORCE_INLINE T & get()
	{
		return *instance;
	}

	/**
	 * Get instance pointer
	 * 
	 * @return Pointer to instance
	 */
	static FORCE_INLINE T * getPtr()
	{
		return instance;
	}
};

template<typename T>
T * Singleton<T>::instance = nullptr;