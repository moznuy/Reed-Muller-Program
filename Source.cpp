// Compiles		in VS2015 and
//				in GCC 5.1.0 with: g++ -static -fno-strict-aliasing -DACMP -lm -s -x c++ -std=c++11 -m32 -Wl,--stack=16777216 -O2 -o 1.exe Source.cpp

#define _CRT_SECURE_NO_WARNINGS // for Visual Studio
#include <vector>
#include <set>
#include <iostream>
#include <ctime>
#include <iterator>
// Apparently some functions is missing from G++ compiler that are present in VS so :
#include <cstdlib>
#include <algorithm>
#include <exception>

using namespace std;

//First it was unsigned char but for n > 7 you need more space
typedef unsigned int byte;

//Concatenation of one vector to self n times
void operator *=(vector<bool> &a, byte n) {
	size_t oldSize;
	for (byte i = 1; i < n; i++) {
		oldSize = a.size();
		a.reserve(2 * oldSize);
		copy_n(a.begin(), oldSize, std::back_inserter(a));
	}
}

// bitwise xor on vectors and a - result
void operator^=(vector<bool> &a, const vector<bool> &b) {
	if (a.size() != b.size())
		throw runtime_error("string lengths unequal");

	for (size_t i = 0; i < a.size(); i++)
		a[i] = a[i] ^ b[i];
}

// Counts number of 1s in a vector
byte norm(const vector<bool> &a) {
	byte cnt = 0;
	for (size_t i = 0; i < a.size(); i++)
		if (a[i] == 1)
			cnt++;
	return cnt;
}

// weighted addition of bit vals at posns 1, 2, 4, 8, 16, 32 ...
// to give a message / grayscale value in range 0 - 63[for H[5]] ...
byte msgval(const vector<bool> &a) {
	byte sum = 0;
	for (size_t i = 0, j = 0, k = a.size(); i < a.size(); j++, k >>= 1, i = (1 << j) - 1)
		sum += (byte)k * (int)a[i]; //(32 * a[0] + 16 * a[1] + 8 * a[3] + 4 * a[7] + 2 * a[15] + a[31]) for H[5]
	return sum;
}

// Fills set with indexes of important bits (info bits)
set<byte> importantBits;
void FillImportantBits(byte size) {
	for (byte i = 0, j = 0; i < size; i = (1 << ++j) - 1)
		importantBits.insert(i);
}

// Marks position of important bits with '|' character
void PrintImportantBits(byte size) {
	for (byte i = 0; i < size; i++) {
		if (importantBits.count(i) == 1)
			printf("|");
		else
			printf(" ");
	}
	printf("\n");
}

// Prints vector
void Print(const vector<bool> &a) {
	for (size_t i = 0; i < a.size(); i++)
		printf("%d", (int)a[i]);
}

// Finds the smallest Hamming distance of testcode from all of the codewords in H[N]
byte codematch(const vector<vector<bool> > &cw, const vector<byte> &map, const vector<bool> &testcode, bool diagnotics) {
	byte pos = 0, bestdist = (byte)testcode.size(), dist;
	vector<bool> tmp;
	for (size_t i = 0; i < cw.size(); i++) {
		tmp = cw[map[i]];
		tmp ^= testcode;						//bitwise xor
		dist = norm(tmp);
		if (dist < bestdist) {
			bestdist = dist; pos = (byte)i;
		}
		if (diagnotics)
			printf("grayscale = %d, dist =  %d\n", i, dist);
		else if (bestdist == 0)
			break;
	}
	printf("Input vector is ");
	Print(testcode);
	printf("\nBest  match  is ");
	Print(cw[map[pos]]);
	printf(" at  posn / grayscale = \"%d\", dist = %d\n\n", pos, bestdist);
	return bestdist;
}

// Generates recursively all starting sequences
vector<vector<bool> > getAllH(byte deep) {
	vector<vector<bool> > ret;
	if (deep == 0) {
		ret.push_back({ 0 });
		ret.push_back({ 1 });
		return ret;
	}

	ret = getAllH(deep - 1);

	for (size_t i = 0; i < ret.size(); i++)
		ret[i] *= 2;									// concat
	vector<bool> rowOf0A1;								// row of zeros and ones like in the video
	for (size_t i = 0; i < (1u << (deep - 1)); i++)		// until 2 to the power of deep-1
		rowOf0A1.push_back(0);
	for (size_t i = 0; i < (1u << (deep - 1)); i++)
		rowOf0A1.push_back(1);
	ret.insert(ret.begin() + 1, rowOf0A1);
	return ret;
}

// Generates recursively all bit masks for a specific length like :
// 000001      OR 000011 000110 001100 011000 110000
// 000010         000101 001010 010100 101000
// 000100         001001 010010 100100 ----->
// 001000         010001 100010 ----->
// 010000         100001 ----->
// 100000         ----->
// For length 1   For length 2 .... and so on
void generateMasks(vector<vector<bool> > &masks, byte len, byte size, vector<bool> &cur, byte start = 0) {
	if (len == 0) {
		masks.push_back(cur);
		return;
	}
	for (byte i = start; i < cur.size(); i++) {
		cur[i] = 1;
		generateMasks(masks, len - 1, size, cur, i + 1);
		cur[i] = 0;
	}
}

// Generates all bit masks of all lengths [0..size]
vector<vector<bool> > generateAllMasks(byte size) {
	vector<vector<bool> > ret;
	vector<bool> empty;
	for (byte i = 0; i <= size; i++) {
		empty.clear();
		empty.resize(size);
		generateMasks(ret, i, 6, empty);
	}
	return ret;
}

// Maps H[N] to all code words
// This function is the slowest (For N>10 it's quite clear delay and for n=13 it's already several minutes)
// Some multithreading is required(maybe devide outer loop into threads)
vector<vector<bool> > mapCodewordsUsingMasks(const vector<vector<bool> > &masks, const vector<vector<bool> > &h) {
	vector<vector<bool> > ret;
	for (size_t i = 0; i < masks.size(); i++) {
		ret.push_back(vector<bool>(h[0].size()));
		for (size_t j = 0; j < masks[i].size(); j++) {
			if (masks[i][j] == 1)
				ret.back() ^= h[j + 1];					//bitwise xor
		}
	}
	return ret;
}

// Determines which number corresponds to which word
// Improvements is required
// For now complexity is O(cw.size() ^ 2) (was N^3 :) but I saw that msgval(cw[i]) can be calculated once)
// Maybe some removal when found
vector<byte> GenetateMapping(const vector<vector<bool> > &cw) {
	vector<byte> ret(cw.size());
	vector<byte> val(cw.size());
	for (size_t i = 0; i < cw.size(); i++)
		val[i] = msgval(cw[i]);
	byte k;
	for (size_t i = 0; i < cw.size(); i++) {
		for (size_t j = 0; j < cw.size(); j++) {
			if (i == val[j]) {
				k = (byte)j;
				break;
			}
		}
		ret[i] = k;
	}
	return ret;
}

int main() {
	srand((unsigned int)time(NULL));
	byte n, tests;

	printf("Welcome to the H[n] (Reed-Muller) test suite\nEnter the n (4 <= n <= 6)\n\
[3 is to small 7 doesn't fit in console but you could try uncomment freopen to switch output to file]\n\
[[For n = 13 program genereted file 134MB big and worked about 5 mins !!!!]]\n : ");
#ifdef _DEBUG
	n = 5;
#else
	if (scanf("%d", &n) < 1)
		return 0;
#endif

	printf("Eneter number of random tests k\n\
[again, if you are not using file probably stick to a small number] : ");
#ifdef _DEBUG
	tests = 5;
#else
	if (scanf("%d", &tests) < 1)
		return 0;
#endif

	//freopen("output.txt", "w", stdout);

	vector<vector<bool> > H = getAllH(n);
	vector<vector<bool> > masks = generateAllMasks((byte)H.size() - 1);
	vector<vector<bool> > cw = mapCodewordsUsingMasks(masks, H);
	vector<byte> map = GenetateMapping(cw);

	printf("\nHere are the H[%d] codewords in ascending order of message bit values \n| - corresponds to information bits\n\n", n);
	FillImportantBits((byte)cw[0].size());
	PrintImportantBits((byte)cw[0].size());
	for (size_t i = 0; i < cw.size(); i++) {
		Print(cw[map[i]]);
		printf(" - %d\n", i);
	}

	printf("\nHere are the %d values contained in msg bits at posns 1, 2, 4, 8, 16, 32, ...\n", cw.size());
	for (size_t i = 0; i < cw.size(); i++)
		printf("%d ", msgval(cw[map[i]]));

	printf("\n\n");
	vector<bool> random;
	byte k, l, z;
	for (byte t = 0; t < tests; t++) {
		k = rand() % (byte)cw.size();        // one of code words
		l = rand() % (byte)cw[0].size() / 4; // small number of mistakes
		random = cw[k];
		for (byte i = 0; i < l; i++) {
			z = rand() % random.size();
			random[z] = !random[z];
		}
		codematch(cw, map, random, false);
	}


#ifdef _OldTestingFromVideo
	vector<byte> test63    = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	vector<byte> test15    = { 0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1 }; //roughly at pos15
	vector<byte> test48    = { 1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0 };
	vector<byte> newtest48 = { 1,0,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0 }; // roughly at 48 but 5 bits damaged
	vector<byte> test0     = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; //zero vector undamaged
	vector<byte> newtest0  = { 0,0,1,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1 }; //zero vector but damaged in 7 places
	vector<byte> random    = { 1,0,1,1,1,0,0,1,0,1,0,0,0,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0 }; // random string of 0s and 1s

	//for (size_t i = 0; i < cw.size(); i++)
	//	printf("i = %d dist = %d\n", i, norm(rowxor(cw[map[i]], newtest48)));

	byte x;
	x = codematch(cw, map, test15, false);
	x = codematch(cw, map, newtest48, false);
	printf("\nShow every stage of brute-force testing for test vector distance 8 from grayshade 44\n");
	x = codematch(cw, map, random, true);
#endif
	return 0;
}