#pragma once

#include "coremin.h"

class BitArray
{
protected:
	/// Data buffer
	ubyte * data;

	/// Number of bits
	uint32 count;

	/// Number of bytes
	uint32 size;

public:
	FORCE_INLINE BitArray()
		: data{nullptr}
		, count{0}
		, size{0} {}
	
	FORCE_INLINE explicit BitArray(uint32 _count)
		: data{nullptr}
		, count{_count}
		, size{((count - 1) >> 3) + 1}
	{
		// Alloc empty buffer
		if (size > 0) data = reinterpret_cast<ubyte*>(gMalloc->malloc(size));
	}

	FORCE_INLINE explicit BitArray(void * src, uint32 _count)
		: data{nullptr}
		, count{_count}
		, size{((count - 1) >> 3) + 1}
	{
		if (src && size > 0)
		{
			// Alloc empty buffer and copy data
			data = reinterpret_cast<ubyte*>(gMalloc->malloc(size));
			PlatformMemory::memcpy(data, src, size);
		}
	}

public:
};