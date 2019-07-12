#include "coremin.h"
#include "math/math.h"
#include "misc/log.h"
#include "containers/bitarray.h"
#include "algorithms/des.h"

Malloc * gMalloc = nullptr;

int32 main()
{
	Memory::createGMalloc();

	srand(clock());

	DES des{DES::std};
	des.numRounds = 16;

	BitArray keys[16];
	des.keySchedule(keys, BitArray("secret!!", 64));
	BitArray ptx("mysecret", 64);
	BitArray ctx = des.encryptBlock(ptx, keys);
	printf("%s\n", ctx.getData());
	printf("%s\n", des.decryptBlock(ctx, keys).getData());

	/* float64 cost = 64.;
	Path optimalPath(&des);

	srand(clock());
	//uint64 v = 0x800000000000000ull;
	//uint64 v = 0x1960000000000000ull;
	char v[] = "\x1b\x60\x00\x00\x00\x00\x00\x00";

	//for (;v != 0; v >>= 1)
	{
		BitArray ptx(&v, 64);

		Path path(&des);
		path.init(BitArray(&v, 64).permute(ptx, des.fp));

		if (path.getTotalCost() < cost)
			path.dfSearch(optimalPath, cost);
	} */
	
	return 0;
}