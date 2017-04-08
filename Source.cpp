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

// bitwise xor on vectors
vector<bool> operator^(const vector<bool> &a, const vector<bool> &b) {
	if (a.size() != b.size())
		throw runtime_error("string lengths unequal");

	vector<bool> ret(a);
	for (size_t i = 0; i < a.size(); i++)
		ret[i] = ret[i] ^ b[i];
	return ret;
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
	if (size > 128)
		return; //for economy space and time
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
	if (a.size() <= 128)
		for (size_t i = 0; i < a.size(); i++)
			printf("%d", (int)a[i]);

	else //for economy space and time
	{
		byte sum = 0;
		printf("0x");
		for (size_t i = 0; i < a.size() / 4; i++) {
			sum = (a[a.size() - (i*4 + 0) - 1] << 0)
				+ (a[a.size() - (i*4 + 1) - 1] << 1)
				+ (a[a.size() - (i*4 + 2) - 1] << 2)
				+ (a[a.size() - (i*4 + 3) - 1] << 3);
			printf("%X", sum);
		}
	}
}

// Finds the smallest Hamming distance of testcode from all of the codewords in H[N]
byte codematch(const vector<vector<bool> > &cw, const vector<byte> &map, const vector<bool> &testcode, bool diagnotics) {
	byte pos = 0, bestdist = (byte)testcode.size(), dist;
	for (size_t i = 0; i < cw.size(); i++) {
		dist = norm(cw[map[i]] ^ testcode);
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

// Generates recursively mask tree like :
// before () - index of node
// in () first - index of parent, second - number of new bit
// after () correlation to a specific mask
// 0(-1, 0)000001     0(-1, 0)000001  6(0, 1)000011 11(1, 2)000110 15(2, 3)001100 18(3, 4)011000 20(4, 5)110000
// 1(-1, 1)000010     1(-1, 1)000010  7(0, 2)000101 12(1, 3)001010 16(2, 4)010100 19(3, 5)101000
// 2(-1, 2)000100     2(-1, 2)000100  8(0, 3)001001 12(1, 4)010010 17(2, 5)100100 ------------->
// 3(-1, 3)001000     3(-1, 3)001000  9(0, 4)010001 14(1, 5)100010 ------------->
// 4(-1, 4)010000     4(-1, 4)010000 10(0, 5)100001 ------------->
// 5(-1, 5)100000     5(-1, 5)100000 ------------->
//                    ------------->
// For len1, size 6   For length 2, size 6 and so on ...
//
// For lenN, size N there are 2^N masks
pair<byte, byte> generateTreeMasks(vector<pair<byte, byte> > &masks, byte len, byte size, byte parrent, byte start) {
	byte index, count = 0;
	if (len == 1) {
		index = (byte)masks.size();
		count = size - start;
		for (byte i = start; i < size; i++)
			masks.push_back(make_pair(parrent, i));
		return make_pair(count, index); // return number of pairs and index of the first one
	}
	pair<byte, byte> prev = generateTreeMasks(masks, len - 1, size, parrent, start);
	index = (byte)masks.size();
	for (byte i = 0; i < prev.first; i++) {
		count += generateTreeMasks(masks, 1, size, prev.second + i, masks[prev.second + i].second + 1).first;
	}
	return make_pair(count, index); // return number of pairs and index of the first one
}

// Frame for generateTreeMasks()
vector<pair<byte, byte> > generateAllMasks(byte size) {
	vector<pair<byte, byte> > ret;
	ret.push_back(make_pair(-1, -1)); // represent mask 000..0
	generateTreeMasks(ret, size, size, -1, 0);
	return ret;
}

// Get code word recursivly if not found previously
const vector<bool> getCodeWord(const vector<pair<byte, byte> > &masks, byte index, const vector<vector<bool> > &h, vector<vector<bool> > &res) {
	if (res[index].size() > 0)
		return res[index];
	if (masks[index].first == -1)
		return h[masks[index].second + 1];

	return getCodeWord(masks, masks[index].first, h, res) ^ h[masks[index].second + 1]; //bitwise xor
}

// Maps H[N] to all code words
vector<vector<bool> > mapCodewordsUsingMasks(const vector<pair<byte, byte> > &masks, const vector<vector<bool> > &h) {
	vector<vector<bool> > ret(masks.size());
	for (size_t i = 0; i < masks.size(); i++)
		ret[i] = getCodeWord(masks, i, h, ret);
	//ret.insert(ret.begin(), h[0]);
	return ret;
}

// Determines which number corresponds to which word
vector<byte> GenetateMapping(const vector<vector<bool> > &cw) {
	vector<byte> ret(cw.size());
	for (size_t i = 0; i < cw.size(); i++)
		ret[msgval(cw[i])] = i;
	return ret;
}

int main() {
	srand((unsigned int)time(NULL));
	byte n, tests;

	printf("Welcome to the H[n] (Reed-Muller) test suite\nEnter the n (4 <= n <= 6)\n\
[3 is to small 7 doesn't fit in console but you could try uncomment freopen to switch output to file]\n\
[[For n = 15 program allocated 280MB in memory, genereted file 525MB big! and worked about 2 minutes! \
For 95%% of the time it was writting the result]]\n\
[[[Size and number of code words is proportional to 2^N so be carefull]]]\n : ");
#ifdef _DEBUG
	n = 5;
#else
	if (scanf("%d", &n) < 1)
		return 0;
#endif

	printf("\nEneter number of random tests k\n\
[again, if you are not using file probably stick to a small number]\n : ");
#ifdef _DEBUG
	tests = 5;
#else
	if (scanf("%d", &tests) < 1)
		return 0;
#endif

	freopen("output.txt", "w", stdout);

	vector<vector<bool> > H = getAllH(n);
	vector<pair<byte, byte> > masksTree = generateAllMasks((byte)H.size() - 1);
	vector<vector<bool> > cw = mapCodewordsUsingMasks(masksTree, H);
	vector<byte> map = GenetateMapping(cw);

	printf("\nHere are the H[%d] codewords in ascending order of message bit values \n| - corresponds to information bits\n\n", n);
	FillImportantBits((byte)cw[0].size());
	PrintImportantBits((byte)cw[0].size());
	for (size_t i = 0; i < cw.size(); i++) {
		Print(cw[map[i]]);
		printf(" - %d\n", i);
	}

	printf("\nTest. %d values should be in the ascending order\n", cw.size());
	int sq = (int)sqrt(cw.size());
	for (size_t i = 0; i < cw.size(); i++) {
		printf("%d ", msgval(cw[map[i]]));
		if ((i + 1) % sq == 0)
			printf("\n");
	}

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