#include "coremin.h"
#include "math/math.h"
#include "containers/sorting.h"

Malloc * gMalloc = nullptr;

Array<int> IP((const int[]){
	57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7,
	56, 48, 40, 32, 24, 16, 8, 0, 58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6}, 64);
Array<int> IPinv((const int[]){
	39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41, 9, 49, 17, 57, 25, 32, 0, 40, 8, 48, 16, 56, 24}, 64);
Array<int> Permut((const int[]){
	15, 6, 19, 20, 28, 11, 27, 16, 0, 14, 22, 25, 4, 17, 30, 9,  1, 7, 23, 13, 31, 26, 2, 8, 18, 12, 29, 5, 21, 10, 3, 24},32);
const unsigned char sboxes[8][64] = {
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

void printBits(const Array<ubyte> &x) {
	for (ubyte bit : x) printf("%u", bit); printf("\n");
}

void printHex(const Array<ubyte> &x) {
	int nBits = x.getCount();
	if (nBits%4 != 0) {
		printf("Error in printHex: bits % 4 != 0.\n");
	}
	int nBytes = nBits/4;
	for (int i=0; i<nBytes; i++) {
		int offset = i*4;
		ubyte out = 8*x[offset+0] + 4*x[offset+1] + 2*x[offset+2] + 1*x[offset+3];
		printf("%X",out);
	}
	printf("\n");
}

void printChars(const Array<ubyte> &x) {
	int nBits = x.getCount();
	if (nBits%8 != 0) {
		printf("Error in printChars: bits % 8 != 0.\n");
	}
	int nChars = nBits/8;
	for (int i=0; i<nChars; i++) {
		int offset = i*8;
		ubyte out = 128*x[offset+0] + 64*x[offset+1] + 32*x[offset+2] + 16*x[offset+3] +
					8*x[offset+4] + 4*x[offset+5] + 2*x[offset+6] + 1*x[offset+7];
		if(isprint(out)) printf("%c",out);
		else printf(".");
	}
	printf("\n");
}

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

Array<ubyte> sbox(const Array<ubyte> &input, int sboxIndex) {
	Array<ubyte> output(4);
	ubyte input_val = 32*input[0] + 16*input[1] + 8*input[2] + 4*input[3] + 2*input[4] + input[5];
	ubyte output_val = sboxes[sboxIndex][input_val];
	ubyte mask = 1;
	for(int j=3; j>=0; j--){
		output.push((output_val & (mask << j)) != 0);
	}
	return output;
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

Array<ubyte> f(const Array<ubyte> &input, const Array<ubyte> &key) {
	Array<ubyte> output(32);
	Array<ubyte> x = xorOp(expand(input),key);
	for(int i=0; i<8; i++){
		Array<ubyte> y = sbox(x(i*6,i*6+6),i);
		output.push(y[0]);
		output.push(y[1]);
		output.push(y[2]);
		output.push(y[3]);
	}
	return permutateChoice(output,Permut);
}

Array<ubyte> encrypt(const Array<ubyte> &message, const Array<Array<ubyte>> &keys) {
	Array<ubyte> m = permutateChoice(message,IP);
	Array<ubyte> L(32), R(32);
	for(int i=0; i<16; i++){
		L = m(0,32);
		R = m(32,64);
		Array<ubyte> oldR = R;
		R = xorOp(L,f(R,keys[i]));
		L = oldR;
		if(i!=15) m = join(L,R);
	}
	m = join(R,L);
	m = permutateChoice(m,IPinv);
	return m;
}

Array<ubyte> decrypt(const Array<ubyte> &message, const Array<Array<ubyte>> &keys) {
	Array<ubyte> m = permutateChoice(message,IP);
	Array<ubyte> L(32), R(32);
	for(int i=15; i>=0; i--){
		L = m(0,32);
		R = m(32,64);
		Array<ubyte> oldR = R;
		R = xorOp(L,f(R,keys[i]));
		L = oldR;
		if(i!=0) m = join(L,R);
	}
	m = join(R,L);
	m = permutateChoice(m,IPinv);
	return m;
}

int numActivatedSBox(const Array<ubyte> &R){
	Array<ubyte> x = expand(R);
	int n=0;
	for(int i=0; i<8; i++){
		int used = 0;
		for(int j=0; j<6; j++) if(x[i*6+j]) used = 1;
		n += used;
	}
	return n;
}

int main()
{
	Memory::createGMalloc();

	char key_chars[] = "\x01\x23\x45\x67\x89\xAB\xCD\xEF";
	Array<ubyte> key = createBitArray(key_chars, 8);
	Array<Array<ubyte>> keys = keySchedule(key);

	char message_chars[] = "CiaoSnep";
	Array<ubyte> m = createBitArray(message_chars, 8);

	printf("Key:     "); printHex(key);
	printf("         "); printChars(key);

	printf("Message: "); printHex(m);
	printf("         "); printChars(m);

	m=encrypt(m,keys);
	//m=decrypt(m,keys);

	printf("Output:  "); printHex(m);
	printf("         "); printChars(m);
	
	/*
	// dX_R sequences that activate just 1 sbox in F1
	char level1_chars[] = "\x02\x04\x06\x20\x40\x60";
	for(int i=0; i<4; i++){
		for(int j=0; j<6; j++){
			char chars[] = "\x00\x00\x00\x00";
			char chars0[] = "\x00\x00\x00\x00";
			chars[i] = level1_chars[j];
			Array<ubyte> m = createBitArray(chars, 4);

			Array<ubyte> n = createBitArray(chars0,4);
			Array<ubyte> x = join(m,n);
			x = permutateChoice(x,IPinv);
			printHex(x);
		}
	}
	*/

	return 0;
}