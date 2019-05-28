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
	/// Default constructor
	FORCE_INLINE BitArray()
		: data{nullptr}
		, count{0}
		, size{0} {}
	
	/// Uninitialized constructor
	FORCE_INLINE explicit BitArray(uint32 _count)
		: data{nullptr}
		, count{_count}
		, size{((count - 1) >> 3) + 1}
	{
		// Alloc empty buffer
		if (size > 0) data = reinterpret_cast<ubyte*>(gMalloc->malloc(size));
	}

	/// Initialized constructor
	FORCE_INLINE BitArray(const void * source, uint32 _count)
		: data{nullptr}
		, count{_count}
		, size{_count ? ((_count - 1) >> 3) + 1 : 0}
	{
		if (source && size > 0)
		{
			// Alloc empty buffer and copy data
			data = reinterpret_cast<ubyte*>(gMalloc->malloc(size));
			PlatformMemory::memcpy(data, source, size);
		}
	}

	/// Copy constructor
	FORCE_INLINE BitArray(const BitArray & other)
		: BitArray(other.data, other.count) {}
	
	/// Move constructor
	FORCE_INLINE BitArray(BitArray && other)
		: data{other.data}
		, count{other.count}
		, size{other.size}
	{
		// Unset temp data
		other.data = nullptr;
	}

	/// Copy assignment
	FORCE_INLINE BitArray & operator=(const BitArray & other)
	{
		if (other.size > size)
		{
			gMalloc->free(data);
			data = reinterpret_cast<ubyte*>(gMalloc->malloc(other.size));
		}

		// Copy data
		count = other.count;
		PlatformMemory::memcpy(data, other.data, (size = other.size));

		return *this;
	}

	/// Move assignment
	FORCE_INLINE BitArray & operator=(BitArray && other)
	{
		if (data) gMalloc->free(data);
		data = other.data;
		count = other.count;
		size = other.size;

		other.data = nullptr;

		return *this;
	}

	/// Destructor
	FORCE_INLINE ~BitArray()
	{
		if (data) gMalloc->free(data);
	}

protected:
	/// Resize if necessary
	FORCE_INLINE bool resizeIfNecessary(uint32 _size)
	{
		return _size > size && (data = reinterpret_cast<ubyte*>(gMalloc->realloc(data, (size = _size))));
	}

	/// Return data buffer with type
	template<typename T>
	FORCE_INLINE T * as() const
	{
		return reinterpret_cast<T*>(data);
	}

public:
	/// Get bit count
	FORCE_INLINE uint32 getCount() const
	{
		return count;
	}

	/// Get data buffer
	template<typename T = ubyte>
	FORCE_INLINE const T * getData() const
	{
		return as<T>();
	}

public:
	/**
	 * Access single bit value
	 * 
	 * @param [in] i bit index
	 * @return bit value as byte
	 */
	FORCE_INLINE ubyte operator[](uint32 i) const
	{
		return (data[i >> 3] >> ((i ^ 0x7) & 0x7)) & 0x1;
	}
	
	/**
	 * Get range as unsigned integer
	 * 
	 * @param [in] begin,end range delimiters
	 * @return unsigned integer
	 */
	FORCE_INLINE uint32 operator()(uint32 begin, uint32 end) const
	{
		uint32 out = 0;

		while (end - begin >= 8)
		{
			const uint32 r = begin >> 3, s = begin & 0x7, t = 8 - s;
			(out <<= t) |= data[r] & ((1U << t) - 1);

			begin += t;
		}

		// Last two Bytes
		if (begin >> 3 != end >> 3)
		{
			uint32 r = begin >> 3, s = 8 - (begin & 0x7), t = end & 0x7;
			out = (data[r] & ((1U << s) - 1)) << t | (data[r + 1] >> (8 - t));
		}
		// Last Byte
		else
		{
			ubyte r = begin >> 3, s = 8 - (begin & 0x7), t = 8 - (end & 0x7);
			(out <<= (end - begin)) |= (data[r] & ((1U << s) - 1)) >> t; 
		}

		return out;
	}

	/**
	 * Bitwise xor compound operator
	 * 
	 * @param [in] other second bitarray operand
	 * @return self
	 */
	FORCE_INLINE BitArray & operator^=(const BitArray & other)
	{
		for (uint32 i = 0; i << 3 < count; ++i)
			data[i] ^= other.data[i];
		
		return *this;
	}

	/**
	 * Bitwise xor operator
	 * 
	 * @param [in] other second operand
	 * @return new bitarray
	 */
	FORCE_INLINE BitArray operator^(const BitArray & other) const
	{
		return BitArray(*this) ^= other;
	}

	/**
	 * Left rotate array in-place (circular shift)
	 * 
	 * ! Does not handle all cases
	 * 
	 * @param [in] offset rotation offset (in bits)
	 * @return self
	 */
	BitArray & rotateLeft(int32 offset)
	{
		while (offset > 0)
		{
			uint32 s = Math::min(offset, 8);

			ubyte u = 0, v = 0;
			for (int32 i = count >> 3; i >= 0; --i)
			{
				v = u, u = data[i] >> (8 - s);
				(data[i] <<= s) |= v;
			}

			data[count >> 3] |= (u << (8 - (count & 0x7)));

			// Next Byte
			offset -= s;
		}

		return *this;
	}

	/**
	 * Shuffle using a permutation table
	 * 
	 * @param [in] dest destination array
	 * @param [in] perm permutation table
	 * @return dest bitarray
	 */
	BitArray & permute(BitArray & dest, const uint32 perm[]) const
	{
		for (uint32 i = 0, k; (k = i << 3) < dest.count; ++i)
		{
			uint32 j = 0; for (; j < 8 && (k + j) < dest.count; ++j)
			{
				const uint32 r = perm[k + j];
				const uint32 s = r >> 3, t = (r ^ 0x7) & 0x7;
				const ubyte x = (data[s] >> t) & 0x1;

				(dest.data[i] <<= 1) |= x;
			}
			
			// Final shift
			dest.data[i] <<= (8 - j);
		}

		return dest;
	}

	/**	
	 * Shuffle using subsitution maps
	 * 
	 * @param [in] dest destination bit stream
	 * @param [in] subs subsitution map(s)
	 * @param [in] n number of sub maps to cycle
	 * @return dest bitstream
	 * @{
	 */
	template<uint8 inSize, uint8 outSize>
	FORCE_INLINE BitArray & substitute(BitArray & dest, const uint32 * const subs[], uint32 numSubs) const
	{
		ubyte * src = data;
		ubyte * dst = dest.data; *dst = 0x0;
		
		uint16 b0 = 0, a0 = 0;
		const uint16 r = (1U << inSize) - 1;

		for (uint32 i = 0, n = 8, m = 8, s = 0; i << 3 < count; ++i, n += 8)
		{
			const uint16 b = src[i] | (b0 << 8);

			while (n >= inSize)
			{
				const ubyte t = (b >> (n -= inSize)) & r;
				const ubyte x = subs[s][t];

				*dst |= x << (m -= outSize);
				if (m <= 0) (*++dst = 0) |= x << (m += 8);

				// Next sbox
				s = ++s == numSubs ? 0 : s;
			}

			b0 = b;
		}

		return dest;
	}
	template<uint32 inSize, uint32 outSize>
	FORCE_INLINE BitArray & substitute(BitArray & dest, const uint32 * subs) const
	{
		return substitute<inSize, outSize>(dest, &subs, 1);
	}
	/// @}

	/**
	 * Returns a copy of a slice of the array
	 * 
	 * @param [in] n number of bits to copy
	 * @param [in] offset offset in bytes
	 * @return array slice
	 */
	FORCE_INLINE BitArray slice(uint32 n, uint32 offset = 0) const
	{
		return BitArray(data + offset, n);
	}
	
	/**
	 * Append an array at the end
	 * 
	 * @param [in] other bitarray to append
	 * @return self
	 */
	BitArray & append(const BitArray & other)
	{
		// Make space for new array
		resizeIfNecessary(((count + other.count - 1) >> 3) + 1);

		ubyte * dst = data;
		ubyte * src = other.data;

		// Fill first gap
		const ubyte r = count & 0x7;
		const ubyte s = 8 - r;
		uint16 u = 0, v = *src >> r;

		if (r)
		{
			(dst[count >> 3] &= ~((1U << s) - 1)) |= v;

			// Copy remaining Bytes
			int32 i = other.count >> 3;
			dst += (count + other.count) >> 3, src += i;
			for (; i >= 0; --i, --src, --dst)
			{
				v = u, u = *src >> r;
				*dst = *src << s | v;
			}
		}
		else
		{
			dst += count >> 3;
			for (uint32 i = 0; i << 3 < other.count; ++i)
				dst[i] = src[i];
		}

		// Update count
		count += other.count;
		return *this;
	}

	/**
	 * Merge two bitarrays
	 * 
	 * @param [in] other second bitarray
	 * @return merged array
	 */
	FORCE_INLINE BitArray merge(const BitArray & other) const
	{
		return BitArray(*this).append(other);
	}
};