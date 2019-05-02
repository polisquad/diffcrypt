#include "coremin.h"
#include "math/math.h"
#include "containers/sorting.h"

Malloc * gMalloc = nullptr;

template<uint32 x = 4, uint32 y = 6>
class EBox
{
protected:
	/// Expansion map


public:
	/// Default constructor
	FORCE_INLINE EBox()
	{}
};

template<uint32 x = 6, uint32 y = 6>
class SBox
{
protected:
	/// Substitution map

public:
	/// Default constructor
	FORCE_INLINE SBox()
	{}
};

FORCE_INLINE Array<ubyte> makeBitArray(void * buffer, sizet count)
{
	Array<ubyte> out(count);

	const uint32 blockCount = 1 + (count >> 6);
	const uint32 blockSize = Math::min<sizet>(1 << 6, count);

	for (uint32 i = 0; i < blockCount; ++i)
	{
		uint64 block = reinterpret_cast<uint64*>(buffer)[i];
		for (uint32 j = 0; j < blockSize; ++j, block >>= 1)
			out.push(block & 0x1);
	}

	return out;
}

Array<ubyte> expand(const Array<ubyte> & input)
{
	Array<ubyte> out(48);

	uint32 emap[8][6] = {
		{31, 0, 1, 2, 3, 4},
		{3, 4, 5, 6, 7, 8},
		{7, 8, 9, 10, 11, 12},
		{11, 12, 13, 14, 15, 16},
		{15, 16, 17, 18, 19, 20},
		{19, 20, 21, 22, 23, 24},
		{23, 24, 25, 26, 27, 28},
		{27, 28, 29, 30, 31, 0}
	};

	for (uint32 i = 0; i < 8; ++i)
		for (uint32 j = 0; j < 6; ++j)
			out.push(input[emap[i][j]]);
	
	return out;
}

int main()
{
	Memory::createGMalloc();
	
	char input[] = "1234";

	auto bits = makeBitArray(input, 32);
	for (auto bit : bits) printf("%u", bit); printf("\n");

	auto eout = expand(bits);
	for (auto bit : eout) printf("%u", bit); printf("\n");

	return 0;
}