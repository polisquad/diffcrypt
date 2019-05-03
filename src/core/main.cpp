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
	FORCE_INLINE void permute(BitStream & dest, const uint32 * table)
	{
		uint64 * src = as<uint64>();
		uint64 * des = dest.as<uint64>();

		int32 j = dest.count - 1;
		for (uint32 i = 0; i < dest.count; des += (++i & 0x3f) == 0, --j)
		{
			uint64	r = table[j],
					s = r >> 6,
					t = r & 0x3f,
					u = 1 << r;
			
			// Set bit and shift
			(*des <<= 1) |= (src[s] & u) >> t;
		}
	}

	/**
	 * Shuffle using a subsitution map
	 * 
	 * @param [in] dest destination bit stream
	 * @param [in] sub subsitution map
	 */
	template<uint32 inSize, uint32 outSize>
	FORCE_INLINE void substitute(BitStream & dest, const uint32 * map)
	{
		uint64 * src = as<uint64>(), * des = dest.as<uint64>();
		uint32 i = 0, j = 0;
		uint32 u = (1 << inSize) - 1, v = (1 << outSize) - 1;

		while (j < dest.count)
		{
			uint64 x = (*src & u) >> i;
			uint64 t = static_cast<uint64>(map[x]) << j;
			(*des &= ~v) |= t;

			// Shift bitmask
			u <<= inSize, v <<= outSize;

			// Increment pointers if necessary
			src += ((i += inSize) & 0x3f) == 0;
			des += ((j += outSize) & 0x3f) == 0;
		}
	}
};

#include <omp.h>
double start;

int main()
{
	Memory::createGMalloc();

	srand(clock());
	
	char * username = "00eppy";
	uint32 key = rand();
	BitStream x(username, 32);
	BitStream y(32);
	BitStream k(&key, 32);

	/* start = omp_get_wtime();
	for (uint32 i = 0; i < 1 << 20; ++i)
		x ^= x;
	printf("%fs\n", omp_get_wtime() - start);

	start = omp_get_wtime();
	for (uint32 i = 0; i < 1 << 20; ++i)
		a ^= a;
	printf("%fs\n", omp_get_wtime() - start);

	printf("%hu\n", a); */

	start = omp_get_wtime();
	for (uint32 i = 0; i < 1 << 8; ++i)
	{
		const uint32 subs[] = {5, 1, 6, 10, 13, 7, 14, 2, 3, 9, 15, 8, 12, 4, 0, 11};
		const uint32 perm[] = {3, 28, 18, 1, 24, 12, 20, 16, 0, 4, 21, 6, 17, 13, 7, 26, 2, 29, 25, 15, 31, 30, 23, 8, 11, 22, 10, 5, 27, 19, 14, 9};
		x.permute(y, perm);
		y ^= k;
		y.substitute<4, 4>(x, subs);
		printf("%08x\n", x.as<uint32>()[0]);
	}
	printf("%fs\n", omp_get_wtime() - start);

	printf("%08x\n", y.as<uint32>()[0]);
	printf("%08x\n", x.as<uint32>()[0]);

	return 0;
}