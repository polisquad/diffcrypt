#pragma once

#include "core_types.h"
#include "templates/is_integral.h"
#include "hal/platform_atomics.h"

template<typename Int = int32>
class ThreadSafeCounter
{
protected:
	/// Underlying counter variable
	volatile Int counter;

public:
	/// Default-constructor, init to zero
	FORCE_INLINE ThreadSafeCounter()
		: counter{0} {}

	/// Copy constructor
	FORCE_INLINE ThreadSafeCounter(const ThreadSafeCounter<Int> & other)
		: counter{other.counter} {}

	/// Value constructor
	FORCE_INLINE ThreadSafeCounter(Int val)
		: counter{val} {}

	/// Copy assignment
	FORCE_INLINE ThreadSafeCounter & operator=(const ThreadSafeCounter<Int> & other)
	{
		counter = other.counter;
	}

	/// Returns current value
	FORCE_INLINE Int get() const { return counter; }

	/// Increment and return new value
	FORCE_INLINE Int increment() { return PlatformAtomics::increment(&counter) + 1; }

	/// Decrement and return new value
	FORCE_INLINE Int decrement() { return PlatformAtomics::decrement(&counter) - 1; }

	/// Increment and return previous value
	FORCE_INLINE Int postIncrement() { return PlatformAtomics::increment(&counter); }

	/// Increment and return previous value
	FORCE_INLINE Int postDecrement() { return PlatformAtomics::decrement(&counter); }

	/// Add to counter
	template<typename U = Int>
	FORCE_INLINE Int add(U val) { return PlatformAtomics::add(&counter, val); }

	/// Subtract from counter
	template<typename U = typename AddSign<Int>::Type>
	FORCE_INLINE Int subtract(U val) { return PlatformAtomics::add(&counter, -val); }
};

/// Type definitions for common used counter types
/// @{
typedef ThreadSafeCounter<int32> ThreadSafeCounter32;
typedef ThreadSafeCounter<int64> ThreadSafeCounter64;
typedef ThreadSafeCounter<uint32> ThreadSafeCounterU32;
typedef ThreadSafeCounter<uint64> ThreadSafeCounterU64;
/// @}

