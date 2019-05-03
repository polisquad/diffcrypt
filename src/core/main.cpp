#include "coremin.h"
#include "math/math.h"
#include "containers/sorting.h"

Malloc * gMalloc = nullptr;

class BitStream
{
protected:
	/// Data buffer
	void * data;

	/// Number of bits
	uint32 count;

	/// Buffer size in bytes
	uint32 size;

public:
	/// Default constructor
	FORCE_INLINE explicit BitStream(uint32 _count = 0)
		: data{nullptr}
		, count{_count}
		, size{(((_count - 1) >> 6) + 1) << 3}
	{
		if (size) data = gMalloc->malloc(size, sizeof(uint64));
	}

	/// Buffer constructor
	FORCE_INLINE explicit BitStream(const void * buffer, uint32 _count)
		: count{_count}
		, size{(((_count - 1) >> 6) + 1) << 3}
	{
		// Copy data
		if ((data = gMalloc->malloc(size, sizeof(uint64))))
			PlatformMemory::memcpy(data, buffer, size);
	}

	/// Copy constructor
	FORCE_INLINE BitStream(const BitStream & other)
		: BitStream(other.data, other.count) {}
	
	/// Move constructor
	FORCE_INLINE BitStream(BitStream && other)
		: data(other.data)
		, count(other.count)
	{
		other.data = nullptr;
	}

	/// Copy assignment
	FORCE_INLINE BitStream & operator=(const BitStream & other)
	{
		count = other.count;
		size = other.size;

		// Copy data
		if (data) gMalloc->free(data);
		if (data = gMalloc->malloc(size, sizeof(uint64)))
			PlatformMemory::memcpy(data, other.data, size);
	}

	/// Move assignment
	FORCE_INLINE BitStream & operator=(BitStream && other)
	{
		count = other.count;
		size = other.size;
		data = other.data;

		other.data = nullptr;
	}

	/// Destructor
	FORCE_INLINE ~BitStream()
	{
		if (data) gMalloc->free(data);
	}

	/// Return data buffer as type
	template<typename T>
	T * as() const
	{
		return reinterpret_cast<T*>(data);
	}

	/**
	 * Xor compound operator
	 * 
	 * @param [in] other second operand
	 * @return self
	 */
	FORCE_INLINE BitStream & operator^=(const BitStream & other)
	{
		uint64 * a = as<uint64>(), * b = other.as<uint64>();
		const uint32 n = size >> 3;

		// 64-bit xor
		for (uint32 i = 0; i < n; ++i)
			a[i] ^= b[i];
		
		return *this;
	}

	/**
	 * Xor operator
	 * 
	 * @param [in] other second operand
	 * @return new bit stream
	 */
	FORCE_INLINE BitStream operator^(const BitStream & other) const
	{
		return BitStream(*this) ^= other;
	}

	/**
	 * Shuffle using a permutation table
	 * 
	 * @param [in] dest destination bit stream
	 * @param [in] perm permutation table
	 */
	FORCE_INLINE void permute(BitStream & dest, const uint32 * perm)
	{
		uint64 * src = as<uint64>();
		uint64 * des = dest.as<uint64>();

		int32 j = dest.count - 1;
		for (uint32 i = 0; i < dest.count; des += (++i & 0x3f) == 0, --j)
		{
			const uint64	r = perm[i],
							s = r >> 6,
							t = r & 0x3f,
							x = (src[s] >> t) & 0x1;
			
			// Set bit and shift
			(*des <<= 1) |= x;
		}
	}

	/**	
	 * Shuffle using subsitution maps
	 * 
	 * @param [in] dest destination bit stream
	 * @param [in] subs subsitution map(s)
	 * @param [in] n number of sub maps to cycle
	 * @{
	 */
	template<uint32 inSize, uint32 outSize>
	FORCE_INLINE void substitute(BitStream & dest, const uint32 ** subs, uint32 n)
	{
		uint64 * src = as<uint64>(), * des = dest.as<uint64>();
		uint32 i = 0, j = 0, s = 0;
		uint32 u = (1U << inSize) - 1, v = (1U << outSize) - 1;

		while (j < dest.count)
		{
			uint64 x = (*src & u) >> i;
			uint64 t = static_cast<uint64>(subs[s][x]) << j;
			(*des &= ~v) |= t;

			// Shift bitmask
			u <<= inSize, v <<= outSize;

			// Increment pointers if necessary
			src += ((i += inSize) & 0x3f) == 0;
			des += ((j += outSize) & 0x3f) == 0;

			// Next map
			s = ++s == n ? 0 : s;
		}
	}
	template<uint32 inSize, uint32 outSize>
	FORCE_INLINE void substitute(BitStream & out, const uint32 * subs)
	{
		substitute<inSize, outSize>(out, &subs, 1);
	}
	/// @}
};

#include <omp.h>
double start;

const uint32 xpns[] = {
	31, 0, 1, 2, 3, 4,
	3, 4, 5, 6, 7, 8,
	7, 8, 9, 10, 11, 12,
	11, 12, 13, 14, 15, 16,
	15, 16, 17, 18, 19, 20,
	19, 20, 21, 22, 23, 24,
	23, 24, 25, 26, 27, 28,
	27, 28, 29, 30, 31, 0
};
const uint32 perm[] = {
	15, 6, 19, 20,
	28, 11, 27, 16,
	0, 14, 22, 25,
	4, 17, 30, 9,
	1, 7, 23, 13,
	31, 26, 2, 8,
	18, 12, 29, 5,
	21, 10, 3, 24
};
const uint32 subs[] = {
	14, 0, 4, 15, 13, 7, 1, 4, 2, 14, 15, 2, 11, 13, 8, 1, 3, 10, 10, 6, 6, 12, 12, 11, 5, 9, 9, 5, 0, 3, 7, 8,
	4, 15, 1, 12, 14, 8, 8, 2, 13, 4, 6, 9, 2, 1, 11, 7, 15, 5, 12, 11, 9, 3, 7, 14, 3, 10, 10, 0, 5, 6, 0, 13
};

int main()
{
	Memory::createGMalloc();
	
	char block[] = "sneppy13";
	char output[] = "sneppy13";
	uint64 key = 0x00012345;
	BitStream l(block, 32), r(block + 4, 32);
	BitStream u(block, 32), v(block, 32);
	BitStream k(&key, 32), e(&key, 48);

	uint32 p2[32];
	for (uint32 i = 0; i < 32; ++i)
		p2[i] = i;

	printf("%08x:%08x\n", l.as<uint32>()[0], r.as<uint32>()[0]);

	for (uint32 i = 0; i < 15; ++i)
	{
		// E-box
		r.permute(e, xpns);

		// Xor
		e ^= k;

		// S-box
		e.substitute<6, 4>(v, subs);

		// P-box
		v.permute(u, perm);

		// Xor
		u ^= l;

		// Swap
		l = r;
		r = u;
	}

	{
		l.permute(e, xpns);
		e ^= k;
		e.substitute<6, 4>(v, subs);
		v.permute(u, perm);
		r = u ^ r;
	}

	printf("%08x:%08x\n", l.as<uint32>()[0], r.as<uint32>()[0]);

	for (uint32 i = 0; i < 15; ++i)
	{
		// E-box
		r.permute(e, xpns);

		// Xor
		e ^= k;

		// S-box
		e.substitute<6, 4>(v, subs);

		// P-box
		v.permute(u, perm);

		// Xor
		u ^= l;

		// Swap
		l = r;
		r = u;
	}

	{
		l.permute(e, xpns);
		e ^= k;
		e.substitute<6, 4>(v, subs);
		v.permute(u, perm);
		r = u ^ r;
	}

	printf("%08x:%08x\n", l.as<uint32>()[0], r.as<uint32>()[0]);

	return 0;
}