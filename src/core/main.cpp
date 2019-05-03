#include "coremin.h"
#include "math/math.h"
#include "containers/sorting.h"

Malloc * gMalloc = nullptr;

/*
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
*/

Array<ubyte> createBitArray(char* buffer, int count) {
	Array<ubyte> out(count);
	ubyte mask = 1;
	for (int i = 0; i < count; ++i) {
		ubyte byte = buffer[i];
		for(int j=7; j>=0; j--){
			out.push((byte & (mask << j)) != 0);
		}
	}
	return out;
}

Array<ubyte> expand(const Array<ubyte> & input) {
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

Array<ubyte> permutateChoice(const Array<ubyte> &input, const Array<int> &pos) {
	const int N = pos.getCount();
	Array<ubyte> output(N);
	for(int i=0; i<N; i++){
		output.push(input[pos[i]]);
	}
	return output;
}

Array<ubyte> xorOp(const Array<ubyte> &a, const Array<ubyte> &b) {
	const int N = a.getCount();
	Array<ubyte> output(N);
	for(int i=0; i<N; i++){
		output.push(a[i] ^ b[i]);
	}
	return output;
}

Array<ubyte> leftShiftRotate(const Array<ubyte> &x, int k) {
	int n = x.getCount();
	int shift = k%n;
	Array<ubyte> out(n);
	for(int i=shift; i<n; i++){
		out.push(x[i]);
	}
	for(int i=0; i<shift; i++){
		out.push(x[i]);
	}
	return out;
}

Array<ubyte> join(const Array<ubyte> &a, const Array<ubyte> &b) {
	int na = a.getCount();
	int nb = b.getCount();
	Array<ubyte> out(na+nb);
	for(int i=0; i<na; i++) out.push(a[i]);
	for(int i=0; i<nb; i++) out.push(b[i]);
	return out;
}

void printBits(const Array<ubyte> &x) {
	for (ubyte bit : x) printf("%u", bit); printf("\n");
}

Array<Array<ubyte>> keySchedule(const Array<ubyte> &key) {
	int v[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
	Array<int> PC1_C0((const int[]){56, 48, 40, 32, 24, 16, 8, 0, 57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35}, 28);
	Array<int> PC1_D0((const int[]){62, 54, 46, 38, 30, 22, 14, 6, 61, 55, 45, 37, 29, 21, 13, 5, 60, 52, 44, 36, 28, 20, 12, 4, 27, 19, 11, 3}, 28);
	Array<int> PC2((const int[]){13, 16, 10, 23, 0, 4, 2, 27, 14, 5, 20, 9, 22, 18, 11, 3, 25, 7, 15, 6, 26, 19, 12, 1, 40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47, 43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31},48);
	Array<ubyte> C0 = permutateChoice(key,PC1_C0);
	Array<ubyte> D0 = permutateChoice(key,PC1_D0);
	Array<Array<ubyte>> keys(16);
	for(int i=0; i<16; i++){
		Array<ubyte> C = leftShiftRotate(C0,v[i]);
		Array<ubyte> D = leftShiftRotate(D0,v[i]);
		Array<ubyte> T = join(C,D);
		keys[i] = permutateChoice(T,PC2);
		C0 = C;
		D0 = D;
	}
	return keys;
}

Array<int> IP((const int[]){57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3, 61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7, 56, 48, 40, 32, 24, 16, 8, 0, 58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6}, 64);
Array<int> IPinv((const int[]){39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25, 32, 0, 40, 8, 48, 16, 56, 24}, 64);

int main()
{
	Memory::createGMalloc();
	
	char key_chars[] = "\x13\x34\x57\x79\x9B\xBC\xDF\xF1";
	Array<ubyte> key = createBitArray(key_chars, 8);
	Array<Array<ubyte>> keys = keySchedule(key);

	char message_chars[] = "\xA3\x5F\xBB\xD0\x8E\x66\xAA\x8D";
	Array<ubyte> m = createBitArray(message_chars, 8);

	Array<ubyte> m1 = permutateChoice(m,IP);
	Array<ubyte> L = m1(0,32);
	Array<ubyte> R = m1(32,64);

	

	return 0;
}