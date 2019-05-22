#include "coremin.h"
#include "math/math.h"
#include "containers/sorting.h"
#include "bitarray.h"

const uint32 numRounds = 6;

Malloc * gMalloc = nullptr;

#include <omp.h>
double start;

const uint32 ip[] = {
	57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7,
	56, 48, 40, 32, 24, 16, 8, 0, 58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6
};
const uint32 fp[] = {
	39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41, 9, 49, 17, 57, 25, 32, 0, 40, 8, 48, 16, 56, 24
};
const uint32 xpn[] = {
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
const uint32 subs[8][64] = {
	{
		14, 0, 4, 15, 13, 7, 1, 4, 2, 14, 15, 2, 11, 13, 8, 1,
		3, 10, 10, 6, 6, 12, 12, 11, 5, 9, 9, 5, 0, 3, 7, 8,
		4, 15, 1, 12, 14, 8, 8, 2, 13, 4, 6, 9, 2, 1, 11, 7,
		15, 5, 12, 11, 9, 3, 7, 14, 3, 10, 10, 0, 5, 6, 0, 13
	},
	{
		15, 3, 1, 13, 8, 4, 14, 7, 6, 15, 11, 2, 3, 8, 4, 14,
		9, 12, 7, 0, 2, 1, 13, 10, 12, 6, 0, 9, 5, 11, 10, 5,
		0, 13, 14, 8, 7, 10, 11, 1, 10, 3, 4, 15, 13, 4, 1, 2,
		5, 11, 8, 6, 12, 7, 6, 12, 9, 0, 3, 5, 2, 14, 15, 9
	},
	{
		10, 13, 0, 7, 9, 0, 14, 9, 6, 3, 3, 4, 15, 6, 5, 10,
		1, 2, 13, 8, 12, 5, 7, 14, 11, 12, 4, 11, 2, 15, 8, 1,
		13, 1, 6, 10, 4, 13, 9, 0, 8, 6, 15, 9, 3, 8, 0, 7,
		11, 4, 1, 15, 2, 14, 12, 3, 5, 11, 10, 5, 14, 2, 7, 12
	},
	{
		7, 13, 13, 8, 14, 11, 3, 5, 0, 6, 6, 15, 9, 0, 10, 3,
		1, 4, 2, 7, 8, 2, 5, 12, 11, 1, 12, 10, 4, 14, 15, 9,
		10, 3, 6, 15, 9, 0, 0, 6, 12, 10, 11, 1, 7, 13, 13, 8,
		15, 9, 1, 4, 3, 5, 14, 11, 5, 12, 2, 7, 8, 2, 4, 14
	},
	{
		2, 14, 12, 11, 4, 2, 1, 12, 7, 4, 10, 7, 11, 13, 6, 1,
		8, 5, 5, 0, 3, 15, 15, 10, 13, 3, 0, 9, 14, 8, 9, 6,
		4, 11, 2, 8, 1, 12, 11, 7, 10, 1, 13, 14, 7, 2, 8, 13,
		15, 6, 9, 15, 12, 0, 5, 9, 6, 10, 3, 4, 0, 5, 14, 3
	},
	{
		12, 10, 1, 15, 10, 4, 15, 2, 9, 7, 2, 12, 6, 9, 8, 5,
		0, 6, 13, 1, 3, 13, 4, 14, 14, 0, 7, 11, 5, 3, 11, 8,
		9, 4, 14, 3, 15, 2, 5, 12, 2, 9, 8, 5, 12, 15, 3, 10,
		7, 11, 0, 14, 4, 1, 10, 7, 1, 6, 13, 0, 11, 8, 6, 13
	},
	{
		4, 13, 11, 0, 2, 11, 14, 7, 15, 4, 0, 9, 8, 1, 13, 10,
		3, 14, 12, 3, 9, 5, 7, 12, 5, 2, 10, 15, 6, 8, 1, 6,
		1, 6, 4, 11, 11, 13, 13, 8, 12, 1, 3, 4, 7, 10, 14, 7,
		10, 9, 15, 5, 6, 0, 8, 15, 0, 14, 5, 2, 9, 3, 2, 12
	},
	{
		13, 1, 2, 15, 8, 13, 4, 8, 6, 10, 15, 3, 11, 7, 1, 4,
		10, 12, 9, 5, 3, 6, 14, 11, 5, 0, 0, 14, 12, 9, 7, 2,
		7, 2, 11, 1, 4, 14, 1, 7, 9, 4, 12, 10, 14, 8, 2, 13,
		0, 15, 6, 12, 10, 9, 13, 0, 15, 3, 3, 5, 5, 6, 8, 11
	}
};
const uint32 diffs[8][16 * 64] = {
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 6, 0, 2, 4, 4, 0, 10, 12, 4, 10, 6, 2, 4,
		0, 0, 0, 8, 0, 4, 4, 4, 0, 6, 8, 6, 12, 6, 4, 2,
		14, 4, 2, 2, 10, 6, 4, 2, 6, 4, 4, 0, 2, 2, 2, 0,
		0, 0, 0, 6, 0, 10, 10, 6, 0, 4, 6, 4, 2, 8, 6, 2,
		4, 8, 6, 2, 2, 4, 4, 2, 0, 4, 4, 0, 12, 2, 4, 6,
		0, 4, 2, 4, 8, 2, 6, 2, 8, 4, 4, 2, 4, 2, 0, 12,
		2, 4, 10, 4, 0, 4, 8, 4, 2, 4, 8, 2, 2, 2, 4, 4,
		0, 0, 0, 12, 0, 8, 8, 4, 0, 6, 2, 8, 8, 2, 2, 4,
		10, 2, 4, 0, 2, 4, 6, 0, 2, 2, 8, 0, 10, 0, 2, 12,
		0, 8, 6, 2, 2, 8, 6, 0, 6, 4, 6, 0, 4, 0, 2, 10,
		2, 4, 0, 10, 2, 2, 4, 0, 2, 6, 2, 6, 6, 4, 2, 12,
		0, 0, 0, 8, 0, 6, 6, 0, 0, 6, 6, 4, 6, 6, 14, 2,
		6, 6, 4, 8, 4, 8, 2, 6, 0, 6, 4, 6, 0, 2, 0, 2,
		0, 4, 8, 8, 6, 6, 4, 0, 6, 6, 4, 0, 0, 4, 0, 8,
		2, 0, 2, 4, 4, 6, 4, 2, 4, 8, 2, 2, 2, 6, 8, 8,
		0, 0, 0, 0, 0, 0, 2, 14, 0, 6, 6, 12, 4, 6, 8, 6,
		6, 8, 2, 4, 6, 4, 8, 6, 4, 0, 6, 6, 0, 4, 0, 0,
		0, 8, 4, 2, 6, 6, 4, 6, 6, 4, 2, 6, 6, 0, 4, 0,
		2, 4, 4, 6, 2, 0, 4, 6, 2, 0, 6, 8, 4, 6, 4, 6,
		0, 8, 8, 0, 10, 0, 4, 2, 8, 2, 2, 4, 4, 8, 4, 0,
		0, 4, 6, 4, 2, 2, 4, 10, 6, 2, 0, 10, 0, 4, 6, 4,
		0, 8, 10, 8, 0, 2, 2, 6, 10, 2, 0, 2, 0, 6, 2, 6,
		4, 4, 6, 0, 10, 6, 0, 2, 4, 4, 4, 6, 6, 6, 2, 0,
		0, 6, 6, 0, 8, 4, 2, 2, 2, 4, 6, 8, 6, 6, 2, 2,
		2, 6, 2, 4, 0, 8, 4, 6, 10, 4, 0, 4, 2, 8, 4, 0,
		0, 6, 4, 0, 4, 6, 6, 6, 6, 2, 2, 0, 4, 4, 6, 8,
		4, 4, 2, 4, 10, 6, 6, 4, 6, 2, 2, 4, 2, 2, 4, 2,
		0, 10, 10, 6, 6, 0, 0, 12, 6, 4, 0, 0, 2, 4, 4, 0,
		4, 2, 4, 0, 8, 0, 0, 2, 10, 0, 2, 6, 6, 6, 14, 0,
		0, 2, 6, 0, 14, 2, 0, 0, 6, 4, 10, 8, 2, 2, 6, 2,
		2, 4, 10, 6, 2, 2, 2, 8, 6, 8, 0, 0, 0, 4, 6, 4,
		0, 0, 0, 10, 0, 12, 8, 2, 0, 6, 4, 4, 4, 2, 0, 12,
		0, 4, 2, 4, 4, 8, 10, 0, 4, 4, 10, 0, 4, 0, 2, 8,
		10, 4, 6, 2, 2, 8, 2, 2, 2, 2, 6, 0, 4, 0, 4, 10,
		0, 4, 4, 8, 0, 2, 6, 0, 6, 6, 2, 10, 2, 4, 0, 10,
		12, 0, 0, 2, 2, 2, 2, 0, 14, 14, 2, 0, 2, 6, 2, 4,
		6, 4, 4, 12, 4, 4, 4, 10, 2, 2, 2, 0, 4, 2, 2, 2,
		0, 0, 4, 10, 10, 10, 2, 4, 0, 4, 6, 4, 4, 4, 2, 0,
		10, 4, 2, 0, 2, 4, 2, 0, 4, 8, 0, 4, 8, 8, 4, 4,
		12, 2, 2, 8, 2, 6, 12, 0, 0, 2, 6, 0, 4, 0, 6, 2,
		4, 2, 2, 10, 0, 2, 4, 0, 0, 14, 10, 2, 4, 6, 0, 4,
		4, 2, 4, 6, 0, 2, 8, 2, 2, 14, 2, 6, 2, 6, 2, 2,
		12, 2, 2, 2, 4, 6, 6, 2, 0, 2, 6, 2, 6, 0, 8, 4,
		4, 2, 2, 4, 0, 2, 10, 4, 2, 2, 4, 8, 8, 4, 2, 6,
		6, 2, 6, 2, 8, 4, 4, 4, 2, 4, 6, 0, 8, 2, 0, 6,
		6, 6, 2, 2, 0, 2, 4, 6, 4, 0, 6, 2, 12, 2, 6, 4,
		2, 2, 2, 2, 2, 6, 8, 8, 2, 4, 4, 6, 8, 2, 4, 2,
		0, 4, 6, 0, 12, 6, 2, 2, 8, 2, 4, 4, 6, 2, 2, 4,
		4, 8, 2, 10, 2, 2, 2, 2, 6, 0, 0, 2, 2, 4, 10, 8,
		4, 2, 6, 4, 4, 2, 2, 4, 6, 6, 4, 8, 2, 2, 8, 0,
		4, 4, 6, 2, 10, 8, 4, 2, 4, 0, 2, 2, 4, 6, 2, 4,
		0, 8, 16, 6, 2, 0, 0, 12, 6, 0, 0, 0, 0, 8, 0, 6,
		2, 2, 4, 0, 8, 0, 0, 0, 14, 4, 6, 8, 0, 2, 14, 0,
		2, 6, 2, 2, 8, 0, 2, 2, 4, 2, 6, 8, 6, 4, 10, 0,
		2, 2, 12, 4, 2, 4, 4, 10, 4, 4, 2, 6, 0, 2, 2, 4,
		0, 6, 2, 2, 2, 0, 2, 2, 4, 6, 4, 4, 4, 6, 10, 10,
		6, 2, 2, 4, 12, 6, 4, 8, 4, 0, 2, 4, 2, 4, 4, 0,
		6, 4, 6, 4, 6, 8, 0, 6, 2, 2, 6, 2, 2, 6, 4, 0,
		2, 6, 4, 0, 0, 2, 4, 6, 4, 6, 8, 6, 4, 4, 6, 2,
		0, 10, 4, 0, 12, 0, 4, 2, 6, 0, 4, 12, 4, 4, 2, 0,
		0, 8, 6, 2, 2, 6, 0, 8, 4, 4, 0, 4, 0, 12, 4, 4,
		4, 8, 2, 2, 2, 4, 4, 14, 4, 2, 0, 2, 0, 8, 4, 4,
		4, 8, 4, 2, 4, 0, 2, 4, 4, 2, 4, 8, 8, 6, 2, 2
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 4, 0, 2, 6, 4, 0, 14, 8, 6, 8, 4, 6, 2,
		0, 0, 0, 2, 0, 4, 6, 4, 0, 0, 4, 6, 10, 10, 12, 6,
		4, 8, 4, 8, 4, 6, 4, 2, 4, 2, 2, 4, 6, 2, 0, 4,
		0, 0, 0, 0, 0, 6, 0, 14, 0, 6, 10, 4, 10, 6, 4, 4,
		2, 0, 4, 8, 2, 4, 6, 6, 2, 0, 8, 4, 2, 4, 10, 2,
		0, 12, 6, 4, 6, 4, 6, 2, 2, 10, 2, 8, 2, 0, 0, 0,
		4, 6, 6, 4, 2, 4, 4, 2, 6, 4, 2, 4, 4, 6, 0, 6,
		0, 0, 0, 4, 0, 4, 0, 8, 0, 10, 16, 6, 6, 0, 6, 4,
		14, 2, 4, 10, 2, 8, 2, 6, 2, 4, 0, 0, 2, 2, 2, 4,
		0, 6, 6, 2, 10, 4, 10, 2, 6, 2, 2, 4, 2, 2, 4, 2,
		6, 2, 2, 0, 2, 4, 6, 2, 10, 2, 0, 6, 6, 4, 4, 8,
		0, 0, 0, 4, 0, 14, 0, 10, 0, 6, 2, 4, 4, 8, 6, 6,
		6, 2, 6, 2, 10, 2, 0, 4, 0, 10, 4, 2, 8, 2, 2, 4,
		0, 6, 12, 8, 0, 4, 2, 0, 8, 2, 4, 4, 6, 2, 0, 6,
		0, 8, 2, 0, 6, 6, 8, 2, 4, 4, 4, 6, 8, 0, 4, 2,
		0, 0, 0, 8, 0, 4, 10, 2, 0, 2, 8, 10, 0, 10, 6, 4,
		6, 6, 4, 6, 4, 0, 6, 4, 8, 2, 10, 2, 2, 4, 0, 0,
		0, 6, 2, 6, 2, 4, 12, 4, 6, 4, 0, 4, 4, 6, 2, 2,
		4, 0, 4, 0, 8, 6, 6, 0, 0, 2, 0, 6, 4, 8, 2, 14,
		0, 6, 6, 4, 10, 0, 2, 12, 6, 2, 2, 2, 4, 4, 2, 2,
		6, 8, 2, 0, 8, 2, 0, 2, 2, 2, 2, 2, 2, 14, 10, 2,
		0, 8, 6, 4, 2, 2, 4, 2, 6, 4, 6, 2, 6, 0, 6, 6,
		6, 4, 8, 6, 4, 4, 0, 4, 6, 2, 4, 4, 4, 2, 4, 2,
		0, 6, 4, 6, 10, 4, 0, 2, 4, 8, 0, 0, 4, 8, 2, 6,
		2, 4, 6, 4, 4, 2, 4, 2, 6, 4, 6, 8, 0, 6, 4, 2,
		0, 6, 8, 4, 2, 4, 2, 2, 8, 2, 2, 6, 2, 4, 4, 8,
		0, 6, 4, 4, 0, 12, 6, 4, 2, 2, 2, 4, 4, 2, 10, 2,
		0, 4, 6, 6, 12, 0, 4, 0, 10, 2, 6, 2, 0, 0, 10, 2,
		0, 6, 2, 2, 6, 0, 4, 16, 4, 4, 2, 0, 0, 4, 6, 8,
		0, 4, 8, 2, 10, 6, 6, 0, 8, 4, 0, 2, 4, 4, 0, 6,
		4, 2, 6, 6, 2, 2, 2, 4, 8, 6, 10, 6, 4, 0, 0, 2,
		0, 0, 0, 2, 0, 12, 10, 4, 0, 0, 0, 2, 14, 2, 8, 10,
		0, 4, 6, 8, 2, 10, 4, 2, 2, 6, 4, 2, 6, 2, 0, 6,
		4, 12, 8, 4, 2, 2, 0, 0, 2, 8, 8, 6, 0, 6, 0, 2,
		8, 2, 0, 2, 8, 4, 2, 6, 4, 8, 2, 2, 6, 4, 2, 4,
		10, 4, 0, 0, 0, 4, 0, 2, 6, 8, 6, 10, 8, 0, 2, 4,
		6, 0, 12, 2, 8, 6, 10, 0, 0, 8, 2, 6, 0, 0, 2, 2,
		2, 2, 4, 4, 2, 2, 10, 14, 2, 0, 4, 2, 2, 4, 6, 4,
		6, 0, 0, 2, 6, 4, 2, 4, 4, 4, 8, 4, 8, 0, 6, 6,
		8, 0, 8, 2, 4, 12, 2, 0, 2, 6, 2, 0, 6, 2, 0, 10,
		0, 2, 4, 10, 2, 8, 6, 4, 0, 10, 0, 2, 10, 0, 2, 4,
		4, 0, 4, 8, 6, 2, 4, 4, 6, 6, 2, 6, 2, 2, 4, 4,
		2, 2, 6, 4, 0, 2, 2, 6, 2, 8, 8, 4, 4, 4, 8, 2,
		10, 6, 8, 6, 0, 6, 4, 4, 4, 2, 4, 4, 0, 0, 2, 4,
		2, 2, 2, 4, 0, 0, 0, 2, 8, 4, 4, 6, 10, 2, 14, 4,
		2, 4, 0, 2, 10, 4, 2, 0, 2, 2, 6, 2, 8, 8, 10, 2,
		12, 4, 6, 8, 2, 6, 2, 8, 0, 4, 0, 2, 0, 8, 2, 0,
		0, 4, 0, 2, 4, 4, 8, 6, 10, 6, 2, 12, 0, 0, 0, 6,
		0, 10, 2, 0, 6, 2, 10, 2, 6, 0, 2, 0, 6, 6, 4, 8,
		8, 4, 6, 0, 6, 4, 4, 8, 4, 6, 8, 0, 2, 2, 2, 0,
		2, 2, 6, 10, 2, 0, 0, 6, 4, 4, 12, 8, 4, 2, 2, 0,
		0, 12, 6, 4, 6, 0, 4, 4, 4, 0, 4, 6, 4, 2, 4, 4,
		0, 12, 4, 6, 2, 4, 4, 0, 10, 0, 0, 8, 0, 8, 0, 6,
		8, 2, 4, 0, 4, 0, 4, 2, 0, 8, 4, 2, 6, 16, 2, 2,
		6, 2, 2, 2, 6, 6, 4, 8, 2, 2, 6, 2, 2, 2, 4, 8,
		0, 8, 8, 10, 6, 2, 2, 0, 4, 0, 4, 2, 4, 0, 4, 10,
		0, 2, 0, 0, 8, 0, 10, 4, 10, 0, 8, 4, 4, 4, 4, 6,
		4, 0, 2, 8, 4, 2, 2, 2, 4, 8, 2, 0, 4, 10, 10, 2,
		16, 4, 4, 2, 8, 2, 2, 6, 4, 4, 4, 2, 0, 2, 2, 2,
		0, 2, 6, 2, 8, 4, 6, 0, 10, 2, 2, 4, 4, 10, 4, 0,
		0, 16, 10, 2, 4, 2, 4, 2, 8, 0, 0, 8, 0, 6, 2, 0,
		4, 4, 0, 10, 2, 4, 2, 14, 4, 2, 6, 6, 0, 0, 6, 0,
		4, 0, 0, 2, 0, 8, 2, 4, 0, 2, 4, 4, 4, 14, 10, 6
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 2, 0, 4, 2, 12, 0, 14, 0, 4, 8, 2, 6, 10,
		0, 0, 0, 2, 0, 2, 0, 8, 0, 4, 12, 10, 4, 6, 8, 8,
		8, 6, 10, 4, 8, 6, 0, 6, 4, 4, 0, 0, 0, 4, 2, 2,
		0, 0, 0, 4, 0, 2, 4, 2, 0, 12, 8, 4, 6, 8, 10, 4,
		6, 2, 4, 8, 6, 10, 6, 2, 2, 8, 2, 0, 2, 0, 4, 2,
		0, 10, 6, 6, 10, 0, 4, 12, 2, 4, 0, 0, 6, 4, 0, 0,
		2, 0, 0, 4, 4, 4, 4, 2, 10, 4, 4, 8, 4, 4, 4, 6,
		0, 0, 0, 10, 0, 4, 4, 6, 0, 6, 6, 6, 6, 0, 8, 8,
		10, 2, 0, 2, 10, 4, 6, 2, 0, 6, 0, 4, 6, 2, 4, 6,
		0, 10, 6, 0, 14, 6, 4, 0, 4, 6, 6, 0, 4, 0, 2, 2,
		2, 6, 2, 10, 2, 2, 4, 0, 4, 2, 6, 0, 2, 8, 14, 0,
		0, 0, 0, 8, 0, 12, 12, 4, 0, 8, 0, 4, 2, 10, 2, 2,
		8, 2, 8, 0, 0, 4, 2, 0, 2, 8, 14, 2, 6, 2, 4, 2,
		0, 4, 4, 2, 4, 2, 4, 4, 10, 4, 4, 4, 4, 4, 2, 8,
		4, 6, 4, 6, 2, 2, 4, 8, 6, 2, 6, 2, 0, 6, 2, 4,
		0, 0, 0, 4, 0, 12, 4, 8, 0, 4, 2, 6, 2, 14, 0, 8,
		8, 2, 2, 6, 4, 0, 2, 0, 8, 4, 12, 2, 10, 0, 2, 2,
		0, 2, 8, 2, 4, 8, 0, 8, 8, 0, 2, 2, 4, 2, 14, 0,
		4, 4, 12, 0, 2, 2, 2, 10, 2, 2, 2, 2, 4, 4, 4, 8,
		0, 6, 4, 4, 6, 4, 6, 2, 8, 6, 6, 2, 2, 0, 0, 8,
		4, 8, 2, 8, 2, 4, 8, 0, 4, 2, 2, 2, 2, 6, 8, 2,
		0, 6, 10, 2, 8, 4, 2, 0, 2, 2, 2, 8, 4, 6, 4, 4,
		0, 6, 6, 0, 6, 2, 4, 4, 6, 2, 2, 10, 6, 8, 2, 0,
		0, 8, 4, 6, 6, 0, 6, 2, 4, 0, 4, 2, 10, 0, 6, 6,
		4, 2, 4, 8, 4, 2, 10, 2, 2, 2, 6, 8, 2, 6, 0, 2,
		0, 8, 6, 4, 4, 0, 6, 4, 4, 8, 0, 10, 2, 2, 2, 4,
		4, 10, 2, 0, 2, 4, 2, 4, 8, 2, 2, 8, 4, 2, 8, 2,
		0, 6, 8, 8, 4, 2, 8, 0, 12, 0, 10, 0, 4, 0, 2, 0,
		0, 2, 0, 6, 2, 8, 4, 6, 2, 0, 4, 2, 4, 10, 0, 14,
		0, 4, 8, 2, 4, 6, 0, 4, 10, 0, 2, 6, 4, 8, 4, 2,
		0, 6, 8, 0, 10, 6, 4, 6, 4, 2, 2, 10, 4, 0, 0, 2,
		0, 0, 0, 0, 0, 4, 4, 8, 0, 2, 2, 4, 10, 16, 12, 2,
		10, 8, 8, 0, 8, 4, 2, 4, 0, 6, 6, 6, 0, 0, 2, 0,
		12, 6, 4, 4, 2, 4, 10, 2, 0, 4, 4, 2, 4, 4, 0, 2,
		2, 2, 0, 6, 0, 2, 4, 0, 4, 12, 4, 2, 6, 4, 8, 8,
		4, 8, 2, 12, 6, 4, 2, 10, 2, 2, 2, 4, 2, 0, 4, 0,
		6, 0, 2, 0, 8, 2, 0, 2, 8, 8, 2, 2, 4, 4, 10, 6,
		6, 2, 0, 4, 4, 0, 4, 0, 4, 2, 14, 0, 8, 10, 0, 6,
		0, 2, 4, 16, 8, 6, 6, 6, 0, 2, 4, 4, 0, 2, 2, 2,
		6, 2, 10, 0, 6, 4, 0, 4, 4, 2, 4, 8, 2, 2, 8, 2,
		0, 2, 8, 4, 0, 4, 0, 6, 4, 10, 4, 8, 4, 4, 4, 2,
		2, 6, 0, 4, 2, 4, 4, 6, 4, 8, 4, 4, 4, 2, 4, 6,
		10, 2, 6, 6, 4, 4, 8, 0, 4, 2, 2, 0, 2, 4, 4, 6,
		10, 4, 6, 2, 4, 2, 2, 2, 4, 10, 4, 4, 0, 2, 6, 2,
		4, 2, 4, 4, 4, 2, 4, 16, 2, 0, 0, 4, 4, 2, 6, 6,
		4, 0, 2, 10, 0, 6, 10, 4, 2, 6, 6, 2, 2, 0, 2, 8,
		8, 2, 0, 0, 4, 4, 4, 2, 6, 4, 6, 2, 4, 8, 4, 6,
		0, 10, 8, 6, 2, 0, 4, 2, 10, 4, 4, 6, 2, 0, 6, 0,
		2, 6, 2, 0, 4, 2, 8, 8, 2, 2, 2, 0, 2, 12, 6, 6,
		2, 0, 4, 8, 2, 8, 4, 4, 8, 4, 2, 8, 6, 2, 0, 2,
		4, 4, 6, 8, 6, 6, 0, 2, 2, 2, 6, 4, 12, 0, 0, 2,
		0, 6, 2, 2, 16, 2, 2, 2, 12, 2, 4, 0, 4, 2, 0, 8,
		4, 6, 0, 10, 8, 0, 2, 2, 6, 0, 0, 6, 2, 10, 2, 6,
		4, 4, 4, 4, 0, 6, 6, 4, 4, 4, 4, 4, 0, 6, 2, 8,
		4, 8, 2, 4, 2, 2, 6, 0, 2, 4, 8, 4, 10, 0, 6, 2,
		0, 8, 12, 0, 2, 2, 6, 6, 2, 10, 2, 2, 0, 8, 0, 4,
		2, 6, 4, 0, 6, 4, 6, 4, 8, 0, 4, 4, 2, 4, 8, 2,
		6, 0, 2, 2, 4, 6, 4, 4, 4, 2, 2, 6, 12, 2, 6, 2,
		2, 2, 6, 0, 0, 10, 4, 8, 4, 2, 4, 8, 4, 4, 0, 6,
		0, 2, 4, 2, 12, 2, 0, 6, 2, 0, 2, 8, 4, 6, 4, 10,
		4, 6, 8, 6, 2, 2, 2, 2, 10, 2, 6, 6, 2, 4, 2, 0,
		8, 6, 4, 4, 2, 10, 2, 0, 2, 2, 4, 2, 4, 2, 10, 2,
		2, 6, 4, 0, 0, 10, 8, 2, 2, 8, 6, 4, 6, 2, 0, 4
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 16, 16, 0, 0, 16, 16, 0, 0, 0, 0, 0,
		0, 0, 0, 8, 0, 4, 4, 8, 0, 4, 4, 8, 8, 8, 8, 0,
		8, 6, 2, 0, 2, 4, 8, 2, 6, 0, 4, 6, 0, 6, 2, 8,
		0, 0, 0, 8, 0, 0, 12, 4, 0, 12, 0, 4, 8, 4, 4, 8,
		4, 2, 2, 8, 2, 12, 0, 2, 2, 0, 12, 2, 8, 2, 2, 4,
		0, 8, 8, 4, 8, 8, 0, 0, 8, 0, 8, 0, 4, 0, 0, 8,
		4, 2, 6, 4, 6, 0, 16, 6, 2, 0, 0, 2, 4, 2, 6, 4,
		0, 0, 0, 4, 0, 8, 4, 8, 0, 4, 8, 8, 4, 8, 8, 0,
		8, 4, 4, 4, 4, 0, 8, 4, 4, 0, 0, 4, 4, 4, 4, 8,
		0, 6, 6, 0, 6, 4, 4, 6, 6, 4, 4, 6, 0, 6, 6, 0,
		0, 12, 0, 8, 0, 0, 0, 0, 12, 0, 0, 12, 8, 12, 0, 0,
		0, 0, 0, 4, 0, 8, 4, 8, 0, 4, 8, 8, 4, 8, 8, 0,
		8, 4, 4, 4, 4, 0, 0, 4, 4, 8, 0, 4, 4, 4, 4, 8,
		0, 6, 6, 4, 6, 0, 4, 6, 6, 4, 0, 6, 4, 6, 6, 0,
		0, 6, 6, 4, 6, 4, 0, 6, 6, 0, 4, 6, 4, 6, 6, 0,
		0, 0, 0, 0, 0, 8, 12, 4, 0, 12, 8, 4, 0, 4, 4, 8,
		4, 2, 2, 16, 2, 4, 0, 2, 2, 0, 4, 2, 16, 2, 2, 4,
		0, 0, 0, 8, 0, 4, 4, 8, 0, 4, 4, 8, 8, 8, 8, 0,
		8, 2, 6, 0, 6, 4, 0, 6, 2, 8, 4, 2, 0, 2, 6, 8,
		0, 8, 8, 0, 8, 0, 8, 0, 8, 8, 0, 0, 0, 0, 0, 16,
		8, 4, 4, 0, 4, 8, 0, 4, 4, 0, 8, 4, 0, 4, 4, 8,
		0, 8, 8, 4, 8, 8, 0, 0, 8, 0, 8, 0, 4, 0, 0, 8,
		4, 6, 2, 4, 2, 0, 0, 2, 6, 16, 0, 6, 4, 6, 2, 4,
		0, 8, 8, 8, 8, 4, 0, 0, 8, 0, 4, 0, 8, 0, 0, 8,
		4, 4, 4, 0, 4, 4, 16, 4, 4, 0, 4, 4, 0, 4, 4, 4,
		0, 6, 6, 4, 6, 0, 4, 6, 6, 4, 0, 6, 4, 6, 6, 0,
		0, 6, 6, 4, 6, 4, 0, 6, 6, 0, 4, 6, 4, 6, 6, 0,
		0, 8, 8, 8, 8, 4, 0, 0, 8, 0, 4, 0, 8, 0, 0, 8,
		4, 4, 4, 0, 4, 4, 0, 4, 4, 16, 4, 4, 0, 4, 4, 4,
		0, 6, 6, 0, 6, 4, 4, 6, 6, 4, 4, 6, 0, 6, 6, 0,
		0, 0, 12, 8, 12, 0, 0, 12, 0, 0, 0, 0, 8, 0, 12, 0,
		0, 0, 0, 8, 0, 0, 0, 12, 0, 0, 0, 12, 8, 12, 12, 0,
		0, 4, 8, 0, 8, 4, 8, 8, 4, 0, 4, 4, 0, 4, 8, 0,
		8, 2, 2, 0, 2, 4, 8, 6, 2, 8, 4, 6, 0, 6, 6, 0,
		4, 6, 2, 8, 2, 4, 0, 2, 6, 0, 4, 6, 8, 6, 2, 4,
		0, 6, 6, 4, 6, 4, 0, 6, 6, 0, 4, 6, 4, 6, 6, 0,
		0, 8, 4, 4, 4, 0, 0, 4, 8, 8, 0, 8, 4, 8, 4, 0,
		0, 6, 6, 0, 6, 4, 8, 2, 6, 8, 4, 2, 0, 2, 2, 8,
		4, 6, 2, 8, 2, 4, 0, 2, 6, 0, 4, 6, 8, 6, 2, 4,
		16, 4, 4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 0,
		0, 6, 2, 8, 2, 4, 0, 2, 6, 8, 4, 6, 8, 6, 2, 0,
		0, 2, 2, 16, 2, 4, 4, 2, 2, 4, 4, 2, 16, 2, 2, 0,
		8, 0, 4, 0, 4, 8, 16, 4, 0, 0, 8, 0, 0, 0, 4, 8,
		8, 4, 4, 4, 4, 0, 8, 4, 4, 8, 0, 4, 4, 4, 4, 0,
		4, 2, 6, 4, 6, 8, 0, 6, 2, 0, 8, 2, 4, 2, 6, 4,
		16, 0, 0, 0, 0, 16, 0, 0, 0, 0, 16, 0, 0, 0, 0, 16,
		16, 0, 0, 0, 0, 0, 16, 0, 0, 16, 0, 0, 0, 0, 0, 16,
		0, 6, 6, 4, 6, 4, 0, 6, 6, 0, 4, 6, 4, 6, 6, 0,
		0, 8, 4, 4, 4, 0, 0, 4, 8, 8, 0, 8, 4, 8, 4, 0,
		16, 6, 6, 4, 6, 0, 4, 2, 6, 4, 0, 2, 4, 2, 2, 0,
		0, 2, 6, 4, 6, 8, 8, 6, 2, 0, 8, 2, 4, 2, 6, 0,
		0, 12, 12, 8, 12, 0, 0, 0, 12, 0, 0, 0, 8, 0, 0, 0,
		0, 4, 8, 0, 8, 4, 8, 8, 4, 0, 4, 4, 0, 4, 8, 0,
		0, 2, 2, 4, 2, 0, 4, 6, 2, 4, 0, 6, 4, 6, 6, 16,
		0, 2, 6, 4, 6, 8, 8, 6, 2, 0, 8, 2, 4, 2, 6, 0,
		0, 4, 4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 16,
		0, 6, 2, 8, 2, 4, 0, 2, 6, 8, 4, 6, 8, 6, 2, 0,
		0, 4, 4, 0, 4, 8, 8, 4, 4, 8, 8, 4, 0, 4, 4, 0,
		16, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 16,
		0, 4, 4, 4, 4, 0, 8, 4, 4, 8, 0, 4, 4, 4, 4, 8,
		4, 2, 6, 4, 6, 8, 0, 6, 2, 0, 8, 2, 4, 2, 6, 4,
		0, 2, 2, 8, 2, 12, 4, 2, 2, 4, 12, 2, 8, 2, 2, 0,
		8, 4, 0, 8, 0, 0, 0, 0, 4, 16, 0, 4, 8, 4, 0, 8
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 4, 0, 10, 8, 6, 0, 4, 2, 2, 12, 10, 2, 4,
		0, 0, 0, 4, 0, 10, 6, 4, 0, 6, 4, 2, 4, 8, 10, 6,
		8, 2, 4, 6, 4, 4, 2, 2, 6, 8, 6, 4, 4, 0, 2, 2,
		0, 0, 0, 8, 0, 4, 10, 6, 0, 6, 6, 4, 8, 6, 0, 6,
		12, 2, 0, 4, 0, 4, 8, 2, 4, 0, 16, 2, 0, 2, 0, 8,
		0, 8, 4, 6, 4, 6, 2, 2, 4, 4, 6, 0, 6, 0, 2, 10,
		2, 0, 4, 8, 4, 2, 6, 6, 2, 8, 6, 2, 2, 0, 6, 6,
		0, 0, 0, 2, 0, 8, 10, 4, 0, 4, 10, 4, 8, 4, 4, 6,
		8, 6, 0, 4, 0, 6, 6, 2, 2, 10, 2, 8, 6, 2, 0, 2,
		0, 6, 8, 6, 0, 8, 0, 0, 8, 10, 4, 2, 8, 0, 0, 4,
		4, 2, 2, 4, 8, 10, 6, 4, 2, 6, 2, 2, 6, 2, 2, 2,
		0, 0, 0, 10, 0, 2, 10, 2, 0, 6, 10, 6, 6, 6, 2, 4,
		10, 4, 2, 2, 0, 6, 16, 0, 0, 2, 10, 2, 2, 4, 0, 4,
		0, 6, 4, 8, 4, 6, 10, 2, 4, 4, 4, 2, 4, 0, 2, 4,
		4, 4, 0, 8, 0, 2, 0, 2, 8, 2, 4, 2, 8, 4, 4, 12,
		0, 0, 0, 0, 0, 4, 4, 12, 0, 2, 8, 10, 4, 6, 12, 2,
		6, 6, 10, 10, 4, 0, 2, 6, 2, 4, 0, 6, 2, 4, 2, 0,
		0, 2, 4, 2, 10, 4, 0, 10, 8, 6, 0, 6, 0, 6, 6, 0,
		0, 0, 6, 2, 8, 0, 0, 4, 4, 6, 2, 8, 2, 8, 10, 4,
		0, 12, 2, 6, 4, 0, 4, 4, 8, 4, 4, 4, 6, 2, 4, 0,
		4, 8, 0, 2, 8, 0, 2, 4, 2, 2, 4, 2, 4, 8, 8, 6,
		0, 6, 10, 2, 14, 0, 2, 2, 4, 4, 0, 6, 0, 4, 6, 4,
		0, 6, 8, 4, 8, 4, 0, 2, 8, 4, 0, 2, 2, 8, 6, 2,
		0, 10, 8, 0, 6, 4, 0, 4, 4, 4, 6, 4, 4, 4, 0, 6,
		0, 4, 6, 2, 4, 4, 2, 6, 4, 2, 2, 4, 12, 2, 10, 0,
		0, 2, 16, 2, 12, 2, 0, 6, 4, 0, 0, 4, 0, 4, 4, 8,
		2, 8, 12, 0, 0, 2, 2, 6, 8, 4, 0, 6, 0, 0, 8, 6,
		0, 10, 2, 6, 6, 6, 6, 4, 8, 2, 0, 4, 4, 4, 2, 0,
		4, 6, 2, 0, 8, 2, 4, 6, 6, 0, 8, 6, 2, 4, 2, 4,
		0, 2, 6, 2, 4, 0, 0, 2, 12, 2, 2, 6, 2, 10, 10, 4,
		0, 6, 8, 4, 8, 8, 0, 6, 6, 2, 0, 6, 0, 6, 2, 2,
		0, 0, 0, 8, 0, 8, 2, 6, 0, 4, 4, 4, 6, 6, 8, 8,
		0, 0, 0, 6, 6, 2, 6, 4, 6, 10, 14, 4, 0, 0, 4, 2,
		14, 4, 0, 10, 0, 2, 12, 2, 2, 2, 10, 2, 0, 0, 2, 2,
		2, 0, 0, 4, 2, 2, 10, 4, 0, 8, 8, 2, 6, 8, 0, 8,
		6, 2, 8, 4, 4, 4, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2,
		6, 0, 0, 8, 2, 8, 2, 6, 6, 4, 2, 2, 4, 2, 6, 6,
		12, 0, 0, 4, 0, 4, 4, 4, 0, 8, 4, 0, 12, 8, 0, 4,
		12, 2, 0, 2, 0, 12, 2, 2, 4, 4, 8, 4, 8, 2, 2, 0,
		2, 8, 4, 6, 2, 4, 6, 0, 6, 6, 4, 0, 2, 2, 2, 10,
		6, 4, 6, 8, 8, 4, 6, 2, 0, 0, 2, 2, 10, 0, 2, 4,
		4, 4, 0, 2, 2, 4, 6, 2, 0, 0, 6, 4, 10, 4, 4, 12,
		4, 6, 2, 6, 0, 0, 12, 2, 0, 4, 12, 2, 6, 4, 0, 4,
		8, 6, 2, 6, 4, 8, 6, 0, 4, 4, 0, 2, 6, 0, 6, 2,
		4, 4, 0, 4, 0, 6, 4, 2, 4, 12, 0, 4, 4, 6, 4, 6,
		6, 0, 2, 4, 0, 6, 6, 4, 2, 10, 6, 10, 6, 2, 0, 0,
		10, 4, 0, 2, 2, 6, 10, 2, 0, 2, 2, 4, 6, 2, 2, 10,
		0, 4, 8, 4, 6, 4, 0, 6, 10, 4, 2, 4, 2, 6, 4, 0,
		0, 6, 6, 4, 10, 2, 0, 0, 4, 4, 0, 0, 4, 6, 12, 6,
		4, 6, 0, 2, 6, 4, 6, 0, 6, 0, 4, 6, 4, 10, 6, 0,
		8, 10, 0, 14, 8, 0, 0, 8, 2, 0, 2, 4, 0, 4, 4, 0,
		0, 4, 4, 2, 14, 4, 0, 8, 6, 8, 2, 2, 0, 4, 6, 0,
		0, 4, 16, 0, 8, 4, 0, 4, 4, 4, 0, 8, 0, 4, 4, 4,
		4, 4, 4, 6, 2, 2, 2, 12, 2, 4, 4, 8, 2, 4, 4, 0,
		4, 2, 2, 2, 4, 2, 0, 8, 2, 2, 2, 12, 6, 2, 8, 6,
		0, 4, 8, 4, 12, 0, 0, 8, 10, 2, 0, 0, 0, 4, 2, 10,
		0, 8, 12, 0, 2, 2, 2, 2, 12, 4, 0, 8, 0, 4, 4, 4,
		0, 14, 4, 0, 4, 6, 0, 0, 6, 2, 10, 8, 0, 0, 4, 6,
		0, 2, 2, 2, 4, 4, 8, 6, 8, 2, 2, 2, 6, 14, 2, 0,
		0, 0, 10, 2, 6, 0, 0, 2, 6, 2, 2, 10, 2, 4, 10, 8,
		0, 6, 12, 2, 4, 8, 0, 8, 8, 2, 2, 0, 2, 2, 4, 4,
		4, 4, 10, 0, 2, 4, 8, 8, 2, 2, 0, 2, 6, 8, 4, 0,
		8, 6, 6, 0, 4, 2, 2, 4, 4, 2, 8, 6, 2, 4, 6, 0,
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 6, 0, 2, 6, 2, 0, 4, 2, 4, 6, 16, 14, 2,
		0, 0, 0, 2, 0, 10, 6, 10, 0, 2, 4, 8, 6, 6, 8, 2,
		0, 8, 0, 8, 0, 6, 4, 6, 4, 4, 4, 12, 2, 4, 2, 0,
		0, 0, 0, 8, 0, 0, 8, 0, 0, 6, 8, 10, 2, 4, 10, 8,
		10, 2, 4, 4, 4, 8, 8, 4, 2, 2, 0, 4, 0, 8, 0, 4,
		0, 8, 4, 4, 8, 4, 2, 2, 12, 0, 2, 6, 6, 2, 2, 2,
		6, 6, 4, 0, 2, 10, 2, 2, 2, 2, 6, 6, 8, 0, 6, 2,
		0, 0, 0, 6, 0, 2, 16, 4, 0, 2, 6, 2, 4, 12, 6, 4,
		10, 4, 2, 6, 0, 2, 6, 2, 4, 0, 8, 6, 4, 4, 2, 4,
		0, 14, 4, 4, 0, 2, 2, 2, 10, 4, 4, 4, 6, 4, 2, 2,
		4, 6, 2, 0, 2, 2, 12, 8, 2, 2, 2, 6, 8, 2, 0, 6,
		0, 0, 0, 12, 0, 10, 4, 6, 0, 8, 4, 4, 2, 12, 2, 0,
		12, 0, 2, 10, 6, 4, 4, 2, 4, 2, 6, 0, 2, 6, 0, 4,
		0, 6, 4, 0, 4, 4, 10, 8, 6, 2, 4, 6, 2, 0, 6, 2,
		2, 2, 2, 2, 6, 2, 6, 2, 10, 4, 8, 2, 6, 4, 4, 2,
		0, 0, 0, 8, 0, 8, 0, 12, 0, 4, 2, 6, 8, 4, 6, 6,
		6, 2, 6, 4, 6, 2, 6, 4, 6, 6, 4, 2, 4, 0, 6, 0,
		0, 8, 4, 2, 0, 4, 2, 0, 4, 10, 6, 2, 8, 6, 4, 4,
		6, 6, 12, 0, 12, 2, 0, 6, 6, 2, 0, 4, 0, 2, 4, 2,
		0, 4, 6, 2, 8, 6, 0, 2, 6, 10, 4, 0, 2, 4, 6, 4,
		2, 2, 6, 6, 4, 4, 2, 6, 2, 6, 8, 4, 4, 0, 4, 4,
		0, 4, 14, 6, 8, 4, 2, 6, 2, 0, 2, 0, 4, 2, 0, 10,
		2, 6, 8, 0, 0, 2, 0, 2, 2, 6, 0, 8, 8, 2, 12, 6,
		0, 4, 6, 6, 8, 4, 2, 2, 6, 4, 6, 4, 2, 4, 2, 4,
		2, 6, 0, 2, 4, 4, 4, 6, 4, 8, 6, 4, 2, 2, 6, 4,
		0, 6, 6, 0, 8, 2, 4, 6, 4, 2, 4, 6, 2, 0, 4, 10,
		0, 4, 10, 2, 4, 4, 2, 6, 6, 6, 2, 2, 6, 6, 2, 2,
		0, 0, 8, 2, 12, 2, 6, 2, 8, 6, 6, 2, 4, 0, 4, 2,
		2, 4, 0, 6, 8, 6, 0, 2, 6, 8, 6, 0, 2, 4, 0, 10,
		0, 10, 8, 2, 8, 2, 0, 2, 6, 4, 2, 4, 6, 4, 2, 4,
		0, 6, 6, 8, 6, 4, 2, 4, 4, 2, 2, 0, 2, 4, 2, 12,
		0, 0, 0, 0, 0, 6, 6, 4, 0, 4, 8, 8, 4, 6, 10, 8,
		2, 8, 6, 8, 4, 4, 6, 6, 8, 4, 0, 4, 0, 2, 2, 0,
		16, 2, 4, 6, 2, 4, 2, 0, 6, 4, 8, 2, 0, 2, 2, 4,
		0, 4, 0, 4, 4, 6, 10, 4, 2, 2, 6, 2, 4, 6, 6, 4,
		10, 8, 0, 6, 12, 6, 10, 4, 8, 0, 0, 0, 0, 0, 0, 0,
		0, 2, 4, 2, 0, 4, 4, 0, 4, 0, 10, 10, 4, 10, 6, 4,
		2, 2, 0, 12, 2, 2, 6, 2, 4, 4, 8, 0, 6, 6, 8, 0,
		8, 4, 0, 8, 2, 4, 2, 4, 0, 6, 2, 4, 4, 8, 2, 6,
		6, 8, 4, 6, 0, 4, 2, 2, 4, 8, 2, 6, 4, 2, 2, 4,
		2, 4, 4, 0, 8, 8, 6, 8, 6, 4, 0, 4, 4, 4, 2, 0,
		6, 0, 0, 6, 6, 4, 6, 8, 2, 4, 0, 2, 2, 4, 6, 8,
		12, 0, 4, 0, 0, 4, 2, 2, 2, 6, 10, 6, 10, 2, 4, 0,
		4, 2, 6, 0, 0, 6, 8, 6, 4, 2, 2, 8, 4, 6, 4, 2,
		6, 2, 2, 6, 6, 4, 4, 2, 6, 2, 4, 8, 4, 2, 4, 2,
		4, 6, 2, 4, 2, 4, 4, 2, 4, 2, 4, 6, 4, 10, 4, 2,
		10, 0, 4, 8, 0, 6, 6, 2, 0, 4, 4, 2, 6, 2, 2, 8,
		0, 12, 8, 2, 0, 6, 0, 0, 6, 6, 0, 2, 8, 2, 6, 6,
		2, 6, 10, 4, 2, 2, 2, 4, 6, 0, 2, 6, 0, 2, 4, 12,
		4, 2, 2, 8, 10, 8, 8, 6, 0, 2, 2, 4, 4, 2, 2, 0,
		4, 2, 2, 2, 6, 0, 4, 0, 10, 6, 6, 4, 0, 4, 8, 6,
		0, 4, 4, 2, 6, 4, 0, 4, 6, 2, 6, 4, 2, 8, 0, 12,
		6, 12, 4, 2, 4, 2, 2, 4, 8, 2, 2, 0, 6, 4, 4, 2,
		0, 2, 2, 4, 4, 4, 4, 0, 2, 10, 12, 4, 0, 10, 4, 2,
		10, 2, 2, 6, 14, 2, 2, 6, 2, 0, 4, 6, 2, 0, 4, 2,
		0, 4, 14, 0, 8, 2, 0, 4, 4, 4, 2, 0, 8, 2, 4, 8,
		2, 4, 8, 0, 6, 2, 0, 6, 2, 6, 4, 2, 8, 6, 2, 6,
		8, 4, 0, 4, 6, 2, 0, 4, 6, 8, 6, 0, 6, 0, 4, 6,
		0, 4, 6, 6, 2, 2, 2, 14, 0, 12, 0, 4, 2, 2, 8, 0,
		0, 6, 16, 0, 2, 2, 2, 8, 4, 2, 0, 12, 6, 2, 2, 0,
		0, 6, 2, 2, 2, 6, 8, 2, 4, 2, 6, 2, 6, 2, 4, 10,
		4, 2, 2, 4, 4, 0, 6, 10, 4, 2, 4, 6, 6, 2, 6, 2,
		0, 4, 6, 6, 4, 8, 4, 0, 4, 8, 4, 0, 4, 8, 2, 2
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 2, 0, 4, 4, 14, 0, 12, 4, 6, 2, 6, 6, 4,
		0, 0, 0, 0, 0, 12, 2, 2, 0, 4, 0, 4, 8, 12, 6, 14,
		8, 2, 12, 2, 6, 8, 6, 0, 6, 4, 4, 2, 2, 0, 0, 2,
		0, 0, 0, 8, 0, 4, 4, 8, 0, 8, 8, 12, 2, 6, 2, 2,
		6, 0, 0, 2, 8, 0, 8, 4, 0, 2, 6, 0, 10, 6, 6, 6,
		0, 2, 12, 0, 8, 4, 8, 2, 4, 4, 4, 2, 6, 0, 6, 2,
		4, 6, 4, 12, 0, 4, 2, 0, 0, 14, 2, 6, 4, 0, 0, 6,
		0, 0, 0, 8, 0, 0, 6, 10, 0, 4, 12, 4, 6, 6, 0, 8,
		10, 8, 4, 8, 6, 2, 2, 0, 2, 6, 8, 2, 0, 6, 0, 0,
		0, 10, 6, 2, 12, 2, 4, 0, 4, 4, 6, 4, 4, 0, 0, 6,
		0, 2, 2, 2, 4, 8, 6, 4, 4, 0, 4, 2, 6, 4, 2, 14,
		0, 0, 0, 4, 0, 4, 8, 4, 0, 2, 6, 0, 14, 12, 8, 2,
		6, 6, 2, 4, 2, 6, 4, 6, 6, 4, 8, 8, 0, 2, 0, 0,
		0, 12, 10, 10, 0, 2, 4, 2, 8, 6, 4, 2, 0, 0, 2, 2,
		2, 0, 0, 0, 6, 8, 8, 0, 6, 2, 4, 6, 8, 0, 6, 8,
		0, 0, 0, 4, 0, 2, 8, 6, 0, 6, 4, 10, 8, 4, 8, 4,
		6, 10, 10, 4, 4, 2, 0, 4, 4, 0, 2, 8, 4, 2, 2, 2,
		0, 0, 8, 8, 2, 8, 2, 8, 6, 4, 2, 8, 0, 0, 8, 0,
		4, 4, 2, 2, 8, 6, 0, 2, 2, 2, 0, 4, 6, 8, 14, 0,
		0, 8, 6, 2, 8, 8, 2, 6, 4, 2, 0, 2, 8, 6, 0, 2,
		4, 4, 8, 2, 4, 0, 4, 10, 8, 2, 4, 4, 4, 2, 0, 4,
		0, 6, 10, 2, 2, 2, 2, 4, 10, 8, 2, 2, 0, 4, 10, 0,
		8, 2, 4, 2, 6, 4, 0, 6, 4, 4, 2, 2, 0, 4, 8, 8,
		0, 16, 2, 2, 6, 0, 6, 0, 6, 2, 8, 0, 6, 0, 2, 8,
		0, 8, 0, 2, 4, 4, 10, 4, 8, 0, 6, 4, 2, 6, 2, 4,
		0, 2, 4, 8, 12, 4, 0, 6, 4, 4, 0, 2, 0, 6, 4, 8,
		0, 6, 2, 6, 4, 2, 4, 4, 6, 4, 8, 4, 2, 0, 10, 2,
		0, 8, 4, 4, 2, 6, 6, 6, 6, 4, 6, 8, 0, 2, 0, 2,
		4, 4, 4, 0, 0, 2, 4, 2, 4, 2, 2, 4, 10, 10, 8, 4,
		0, 0, 2, 2, 12, 6, 2, 0, 12, 2, 2, 4, 2, 6, 8, 4,
		2, 2, 10, 14, 2, 4, 2, 4, 4, 6, 0, 2, 4, 8, 0, 0,
		0, 0, 0, 14, 0, 8, 4, 2, 0, 4, 2, 8, 2, 6, 0, 14,
		4, 2, 6, 2, 12, 2, 4, 0, 6, 4, 10, 2, 4, 2, 2, 2,
		10, 6, 0, 2, 4, 4, 10, 0, 4, 0, 12, 2, 8, 0, 0, 2,
		0, 6, 2, 2, 2, 4, 6, 10, 0, 4, 8, 2, 2, 6, 0, 10,
		4, 2, 0, 6, 8, 2, 6, 0, 8, 2, 2, 0, 8, 2, 12, 2,
		2, 0, 2, 16, 2, 4, 6, 4, 6, 8, 2, 4, 0, 6, 0, 2,
		6, 10, 0, 10, 0, 6, 4, 4, 2, 2, 4, 6, 2, 4, 2, 2,
		4, 0, 2, 0, 2, 2, 14, 0, 4, 6, 6, 2, 12, 2, 4, 4,
		14, 4, 6, 4, 4, 6, 2, 0, 6, 6, 2, 2, 4, 0, 2, 2,
		2, 2, 0, 2, 0, 8, 4, 2, 4, 6, 4, 4, 6, 4, 12, 4,
		2, 4, 0, 0, 0, 2, 8, 12, 0, 8, 2, 4, 8, 4, 4, 6,
		16, 6, 2, 4, 6, 10, 2, 2, 2, 2, 2, 2, 4, 2, 2, 0,
		2, 6, 6, 8, 2, 2, 0, 6, 0, 8, 4, 2, 2, 6, 8, 2,
		6, 2, 4, 2, 8, 8, 2, 8, 2, 4, 4, 0, 2, 0, 8, 4,
		2, 4, 8, 0, 2, 2, 2, 4, 0, 2, 8, 4, 14, 6, 0, 6,
		2, 2, 2, 8, 0, 2, 2, 6, 4, 6, 8, 8, 6, 2, 0, 6,
		0, 6, 8, 2, 8, 4, 4, 0, 10, 4, 4, 6, 0, 0, 2, 6,
		0, 8, 4, 0, 6, 2, 2, 6, 6, 0, 0, 2, 6, 4, 8, 10,
		2, 4, 0, 0, 6, 4, 10, 6, 6, 4, 6, 2, 4, 6, 2, 2,
		0, 16, 6, 8, 2, 0, 2, 2, 4, 2, 8, 4, 0, 4, 6, 0,
		0, 4, 14, 8, 2, 2, 2, 4, 16, 2, 2, 2, 0, 2, 0, 4,
		0, 6, 0, 0, 10, 8, 2, 2, 6, 0, 0, 8, 6, 4, 4, 8,
		2, 0, 2, 2, 4, 6, 4, 4, 2, 2, 4, 2, 4, 16, 10, 0,
		6, 6, 6, 8, 4, 2, 4, 4, 4, 0, 6, 8, 2, 4, 0, 0,
		0, 2, 2, 2, 8, 8, 0, 2, 2, 2, 0, 6, 6, 4, 10, 10,
		4, 4, 16, 8, 0, 6, 4, 2, 4, 4, 2, 6, 0, 2, 2, 0,
		16, 6, 4, 0, 2, 0, 2, 6, 0, 4, 8, 10, 0, 0, 4, 2,
		2, 0, 0, 2, 0, 4, 4, 4, 2, 6, 2, 6, 6, 12, 12, 2,
		0, 0, 8, 0, 12, 8, 2, 6, 6, 4, 0, 2, 2, 4, 6, 4,
		2, 4, 12, 2, 2, 2, 0, 4, 6, 10, 2, 6, 4, 2, 0, 6,
		4, 6, 6, 6, 2, 0, 4, 8, 2, 10, 4, 6, 0, 4, 2, 0,
		14, 0, 0, 0, 8, 0, 6, 8, 4, 2, 0, 0, 4, 8, 4, 6
	},
	{
		64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 6, 0, 16, 10, 0, 0, 0, 6, 0, 14, 6, 2, 4,
		0, 0, 0, 8, 0, 10, 4, 2, 0, 10, 2, 4, 8, 8, 6, 2,
		6, 0, 2, 8, 2, 6, 4, 0, 6, 6, 6, 2, 2, 0, 8, 6,
		0, 0, 0, 2, 0, 4, 6, 12, 0, 6, 8, 4, 10, 4, 8, 0,
		4, 10, 6, 0, 0, 2, 6, 0, 4, 10, 4, 6, 8, 2, 0, 2,
		0, 0, 10, 4, 6, 4, 4, 8, 2, 6, 4, 2, 4, 2, 2, 6,
		6, 2, 8, 2, 8, 10, 6, 6, 4, 2, 0, 4, 0, 0, 0, 6,
		0, 0, 0, 4, 0, 6, 4, 2, 0, 8, 6, 10, 8, 2, 2, 12,
		8, 4, 0, 6, 0, 4, 4, 6, 2, 4, 6, 2, 12, 2, 0, 4,
		0, 0, 16, 4, 6, 6, 4, 0, 4, 6, 4, 2, 2, 0, 0, 10,
		2, 8, 0, 6, 2, 6, 0, 4, 4, 10, 0, 2, 10, 2, 6, 2,
		0, 0, 0, 2, 0, 10, 10, 6, 0, 6, 6, 6, 2, 6, 10, 0,
		6, 0, 4, 10, 2, 0, 8, 6, 2, 2, 6, 10, 2, 2, 2, 2,
		0, 0, 6, 8, 4, 8, 0, 2, 10, 6, 2, 4, 6, 2, 4, 2,
		8, 0, 4, 2, 2, 4, 2, 2, 2, 6, 4, 6, 0, 2, 14, 6,
		0, 0, 0, 4, 0, 0, 8, 12, 0, 0, 8, 8, 2, 10, 6, 6,
		0, 6, 4, 6, 2, 2, 6, 6, 4, 6, 4, 6, 0, 4, 4, 4,
		0, 4, 0, 8, 6, 2, 8, 4, 2, 4, 4, 6, 2, 4, 10, 0,
		4, 2, 2, 6, 8, 6, 2, 2, 14, 2, 2, 4, 2, 2, 2, 4,
		0, 16, 4, 2, 6, 0, 2, 6, 4, 0, 4, 6, 4, 6, 4, 0,
		0, 10, 6, 0, 6, 0, 2, 8, 2, 2, 0, 8, 2, 6, 6, 6,
		0, 12, 6, 4, 6, 0, 0, 0, 8, 6, 6, 2, 2, 6, 4, 2,
		0, 6, 8, 0, 6, 2, 4, 6, 6, 0, 2, 6, 4, 4, 2, 8,
		0, 12, 2, 2, 8, 0, 8, 0, 10, 4, 4, 2, 4, 2, 0, 6,
		6, 4, 8, 0, 8, 0, 4, 2, 0, 0, 12, 2, 4, 6, 2, 6,
		0, 4, 6, 2, 8, 8, 0, 4, 8, 0, 0, 0, 6, 2, 0, 16,
		2, 4, 8, 10, 2, 4, 2, 8, 2, 4, 8, 2, 0, 2, 4, 2,
		0, 12, 6, 4, 6, 4, 2, 2, 6, 0, 4, 4, 2, 10, 2, 0,
		8, 6, 0, 0, 10, 0, 0, 8, 10, 4, 2, 2, 2, 8, 4, 0,
		0, 4, 8, 6, 8, 2, 4, 4, 10, 2, 2, 4, 2, 0, 6, 2,
		4, 2, 4, 2, 6, 2, 4, 0, 2, 6, 2, 2, 2, 16, 8, 2,
		0, 0, 0, 16, 0, 4, 0, 0, 0, 14, 6, 4, 2, 0, 4, 14,
		0, 0, 2, 10, 2, 8, 10, 0, 0, 6, 6, 0, 10, 2, 2, 6,
		8, 0, 6, 0, 6, 4, 10, 2, 0, 6, 8, 0, 4, 4, 2, 4,
		4, 8, 0, 6, 0, 4, 8, 6, 2, 2, 10, 4, 8, 0, 0, 2,
		4, 0, 4, 8, 4, 6, 2, 4, 8, 6, 2, 0, 0, 4, 4, 8,
		0, 4, 6, 8, 2, 8, 8, 0, 4, 2, 4, 4, 2, 2, 6, 4,
		2, 6, 0, 6, 4, 4, 4, 6, 6, 0, 4, 4, 10, 4, 2, 2,
		6, 6, 0, 0, 2, 2, 6, 2, 4, 4, 6, 10, 2, 6, 2, 6,
		10, 2, 6, 2, 4, 12, 12, 0, 2, 2, 4, 0, 0, 0, 2, 6,
		4, 0, 0, 14, 2, 10, 4, 2, 8, 6, 4, 0, 4, 2, 2, 2,
		8, 8, 0, 2, 0, 2, 4, 0, 2, 6, 8, 14, 2, 8, 0, 0,
		2, 2, 0, 0, 4, 2, 10, 4, 6, 2, 4, 0, 6, 4, 8, 10,
		2, 6, 6, 2, 4, 6, 2, 0, 2, 6, 4, 0, 6, 4, 10, 4,
		8, 0, 4, 4, 6, 2, 0, 0, 6, 8, 2, 4, 6, 4, 4, 6,
		6, 2, 2, 4, 2, 2, 6, 12, 4, 0, 4, 2, 8, 8, 0, 2,
		8, 12, 4, 6, 6, 4, 2, 2, 2, 2, 4, 2, 2, 4, 0, 4,
		0, 4, 6, 2, 10, 2, 2, 2, 4, 8, 0, 0, 8, 4, 6, 6,
		4, 6, 8, 0, 4, 6, 0, 4, 4, 6, 10, 2, 2, 4, 4, 0,
		6, 6, 6, 2, 4, 6, 0, 2, 0, 6, 8, 2, 2, 6, 6, 2,
		6, 6, 4, 2, 4, 0, 0, 10, 2, 2, 0, 6, 8, 4, 0, 10,
		0, 2, 12, 4, 10, 4, 0, 4, 12, 0, 2, 4, 2, 2, 2, 4,
		6, 4, 4, 0, 10, 0, 0, 4, 10, 0, 0, 4, 2, 8, 8, 4,
		4, 6, 2, 2, 2, 2, 6, 8, 6, 4, 2, 6, 0, 4, 10, 0,
		2, 2, 8, 2, 4, 4, 4, 2, 6, 2, 0, 10, 6, 10, 2, 0,
		0, 4, 8, 4, 2, 6, 6, 2, 4, 2, 2, 4, 6, 4, 4, 6,
		4, 4, 4, 8, 0, 6, 0, 6, 4, 8, 2, 2, 2, 4, 8, 2,
		8, 8, 0, 4, 2, 0, 10, 4, 0, 0, 0, 4, 8, 6, 8, 2,
		8, 2, 6, 4, 4, 4, 4, 0, 6, 4, 4, 6, 4, 4, 4, 0,
		0, 6, 6, 6, 6, 0, 0, 8, 8, 2, 4, 8, 4, 2, 4, 0,
		2, 2, 8, 0, 10, 0, 2, 12, 0, 4, 0, 8, 0, 2, 6, 8,
		6, 4, 0, 0, 4, 4, 0, 10, 6, 2, 6, 12, 2, 4, 0, 4,
		0, 6, 6, 0, 4, 4, 6, 10, 0, 6, 8, 2, 0, 4, 8, 0
	}
};
const ubyte dL[24][9] = {
	"\x00\x40\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x40\x00\x00\x00\x00\x00",
	"\x00\x40\x40\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x00\x00\x40\x00\x00",
	"\x00\x00\x00\x00\x00\x00\x40\x00",
	"\x00\x00\x00\x00\x00\x40\x40\x00",
	"\x00\x10\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x10\x00\x00\x00\x00\x00",
	"\x00\x10\x10\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x00\x00\x10\x00\x00",
	"\x00\x00\x00\x00\x00\x00\x10\x00",
	"\x00\x00\x00\x00\x00\x10\x10\x00",
	"\x00\x04\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x04\x00\x00\x00\x00\x00",
	"\x00\x04\x04\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x00\x00\x04\x00\x00",
	"\x00\x00\x00\x00\x00\x00\x04\x00",
	"\x00\x00\x00\x00\x00\x04\x04\x00",
	"\x00\x01\x00\x00\x00\x00\x00\x00",
	"\x00\x00\x01\x00\x00\x00\x00\x00",
	"\x00\x01\x01\x00\x00\x00\x00\x00",
	"\x00\x00\x00\x00\x00\x01\x00\x00",
	"\x00\x00\x00\x00\x00\x00\x01\x00",
	"\x00\x00\x00\x00\x00\x01\x01\x00"
};

struct Node
{
public:
	/// Representation of an sbox along a path
	struct Path
	{
	public:
		/// Right block
		BitArray dr;

		/// Left block
		BitArray dl;

		/// Sbox output (determines path)
		BitArray dy;

		/// Path estimated value
		float32 v;
	};

public:
	/// Path that leads to this node
	Array<Path> paths;

	/// Path cost
	float32 g;

	/// Heuristics
	float32 h;

public:
	/// Comparison operators (required by @ref BinaryTree)
	/// @{
	FORCE_INLINE bool operator<(const Node & other) const
	{
		return (g + h) < (other.g + other.h);
	}
	FORCE_INLINE bool operator>(const Node & other) const
	{
		return (g + h) > (other.g + other.h);
	}
	/// @}

	/**
	 * Initialize node with input plaintext
	 * ! Initial permutation missing
	 * 
	 * @param ptx initial plaintext
	 * @param ip initial permutation table
	 * @param xpns expansion box table
	 */
	void init(const BitArray & ptx, const uint32 ip[], const uint32 xpn[])
	{
		Path input{
			BitArray(32), // dr
			BitArray((const ubyte[]){0x0, 0x0, 0x0, 0x0}, 32), // dl
			BitArray(32), // dy
			0.f
		};
		BitArray u(64);

		// Apply initial permutation
		ptx.permute(u, ip);
		input.dr = u.slice(32);

		// Add input path
		paths.add(input);

		g = 0.f;
	}

	/**
	 * Recompute node heuristics
	 * 
	 * @param [in] diffs diff tables
	 */
	void computeH(const uint32 xpn[], const uint32 * diffs[], uint32 numRounds)
	{
		// Out probability
		float32 p = 1.f;

		BitArray dx(48);
		paths.getLast().dr.permute(dx, xpn);

		const int32 numLeftRounds = numRounds - paths.getCount() - 2;

		if (numLeftRounds)
		{
			// For each sbox
			for (uint32 i = 0, r = 0; i < 8; ++i, r += 6)
			{
				const uint32 * row = diffs[i] + ((dx(r, r + 6)) << 4);

				// Find max probability
				uint32 max = row[0]; for (uint32 j = 1; j < 16; ++j) max = row[j] > max ? row[j] : max;

				// Update best probability
				p *= max / 64.f;
			}

			for (int32 i = 0; i < numLeftRounds; ++i)
				p *= 0.25f;
		}
		
		h = -log2(p);
	}

protected:
	/// Node internal expansion
	void expand_internal(Array<Node> & out, uint32 i, const BitArray & dx, const BitArray & dy, float32 p, const uint32 xpn[], const uint32 perm[], const uint32 * diffs[]) const
	{
		if (p == 0.f) return;

		if (i < 8)
		{
			uint32 x = dx(i * 6, (i + 1) * 6);
			if (x != 0)
				for (uint32 y = 0; y < 16; ++y)
				{
					const float32 sp = diffs[i][x * 16 + y] / 64.f;
					ubyte _y = y << 4;
					expand_internal(out, i + 1, dx, dy.merge(BitArray(&_y, 4)), p * sp, xpn, perm, diffs);
				}
			else
				expand_internal(out, i + 1, dx, dy.merge(BitArray("\x00", 4)), p, xpn, perm, diffs);
		}
		else
		{
			// Push node with dy
			Node n(*this);
			
			// Set dy for last path
			auto & prev = n.paths.getLast();
			prev.dy = dy;

			// Create next path
			Path next;
			next.dl = prev.dr;
			next.dr = BitArray(32);
			next.dy = BitArray(32);
			dy.permute(next.dr, perm) ^= prev.dl;
			n.paths.add(next);

			// Update g and h
			n.g -= log2(p);
			n.computeH(xpn, diffs, numRounds);

			// Add to output
			out.push(n);
		}
	}

public:	
	/**
	 * Expand node
	 * 
	 * @param [in] diffs differntial tables
	 */
	Array<Node> expand(const uint32 xpn[], const uint32 perm[], const uint32 * diffs[]) const
	{
		Array<Node> out;

		BitArray dx(48);
		BitArray dy(0);

		expand_internal(out, 0, paths.getLast().dr.permute(dx, xpn), dy, 1.f, xpn, perm, diffs);

		return out;
	}
};

int main()
{
	Memory::createGMalloc();

	BinaryTree<Node> decisionTree;

	//////////////////////////////////////////////////
	// Initialization
	//////////////////////////////////////////////////
	const uint32 * _diffs[] = {
		diffs[0], diffs[1], diffs[2], diffs[3],
		diffs[4], diffs[5], diffs[6], diffs[7],
	};

	for (uint32 i = 0; i < 24; ++i)
	{
		Node n;
		n.init(BitArray(dL[i], 64), ip, xpn);
		n.computeH(xpn, _diffs, numRounds);
		decisionTree.insert(n);
	}

	for (;;)
	{
		auto it = decisionTree.begin();
		if (it->paths.getCount() + 2 >= numRounds) break;

		for (auto & node : it->expand(xpn, perm, _diffs))
		{
			//printf("node cost: %f + %f = %f\n", node.g, node.h, node.g + node.h);
			decisionTree.insert(node);
		}

		// Delete expanded node
		decisionTree.remove(it);

		if (decisionTree.getCount() > 1U << 22)
		{
			const uint32 numDeleted = decisionTree.getCount() - (1U << 20);
			auto it = decisionTree.last();
			for (uint32 i = 0; i < numDeleted; ++i, --it)
				decisionTree.remove(it);
		}

		printf("num nodes: %u\n", decisionTree.getCount());
		printf("min cost:  %f,%f\n", decisionTree.begin()->g, decisionTree.begin()->h);
		for (auto p : decisionTree.begin()->paths)
			printf("  - dy: %08x / dr: %08x / dl: %08x\n", p.dy.getData<uint32>()[0], p.dr.getData<uint32>()[0], p.dl.getData<uint32>()[0]);

		printf("\n");
		//getc(stdin);
	}
	
	auto & best = *decisionTree.begin();
	const uint32 lastDR = best.paths.getLast().dr.getData<uint32>()[0];

	const uint32 * _subs[] = {
		subs[0], subs[1], subs[2], subs[3],
		subs[4], subs[5], subs[6], subs[7]
	};
	
	char key[] = "\x01\x23\x45\x67\x89\xab\xcd\xef";
	char ptx[] = "CiaoSnep";

	BitArray input(ptx, 64), output(64);
	BitArray l, r;
	BitArray u(32), v(32);
	BitArray k0(key, 64), e(48);
	BitArray k[16];

	//////////////////////////////////////////////////
	// Key schedule
	//////////////////////////////////////////////////
	
	BitArray c(28), d(28);
	const uint32 shifts[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
	const uint32 kperm[] = {13, 16, 10, 23, 0, 4, 2, 27, 14, 5, 20, 9, 22, 18, 11, 3, 25, 7, 15, 6, 26, 19, 12, 1, 40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47, 43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31}; 

	k0.permute(c, (const uint32[]){56, 48, 40, 32, 24, 16, 8, 0, 57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35});
	k0.permute(d, (const uint32[]){62, 54, 46, 38, 30, 22, 14, 6, 61, 55, 45, 37, 29, 21, 13, 5, 60, 52, 44, 36, 28, 20, 12, 4, 27, 19, 11, 3});

	for (uint32 i = 0; i < 16; ++i)
	{
		new (k + i) BitArray(48);

		c.rotateLeft(shifts[i]);
		d.rotateLeft(shifts[i]);

		c.merge(d).permute(k[i], kperm);
	}

	srand(clock());
	
	uint32 cnt = 0;
	uint32 tot; for (tot = 0; tot < 1 << 24; ++tot)
	{
		uint64 q = uint64(rand()) | uint64(rand()) << 32;
		BitArray ptxs[] = {
			BitArray(&q, 64),
			BitArray(&q, 64)
		};
		ptxs[1] ^= best.paths.getFirst().dr.merge(best.paths.getFirst().dl);
		BitArray ctxs[2];

		// Initial permutation
		//input.permute(output, ip);

		for (uint32 h = 0; h < 2; ++h)
		{
			BitArray & ptx = ptxs[h];

			// Split in left and right blocks
			l = ptx.slice(32), r = ptx.slice(32, 4);

			for (uint32 i = 0; i < numRounds - 2; ++i)
			{
				// DES round
				(r.permute(e, xpn) ^= k[i]).substitute<6, 4>(u, _subs, 8).permute(v, perm) ^= l;
				l = r, r = v;
			}

			ctxs[h] = r;
		}
		
		BitArray bar = ctxs[0] ^ ctxs[1];
		const uint32 foo = bar.getData<uint32>()[0];
		//printf("%08x == %08x (cnt: %u)\n", best.paths[1].dr.getData<uint32>()[0], foo, cnt);
		cnt += (best.paths.getLast().dr.getData<uint32>()[0] == foo);

		if ((tot & 0xffff) == 0) printf("%u) %.10f\n", tot, cnt / (float64)tot);

		// 0.0000070408
		// Last round
		/* l ^= (r.permute(e, xpn) ^= k[15]).substitute<6, 4>(u, _subs, 8).permute(v, perm);
		l.merge(r).permute(output, fp); */

		//printf("0x%llx\n", output.getData<uint64>()[0]);
	}

	printf("%u / %u -> %f\n", cnt, tot, cnt / (float32)tot);

	return 0;
}