// This is a simple script to progressively develop H5
// - the Reed-Muller code used on the Mariner 9 mission
// The script does not read in any data.Get the effects you
// want by un - commenting the appropriate sections.Run it via :
// Note: this script is for testing out ideas and for generating lists
// of codewords.A brute - force decoder *is* included but at ~1ms per codeword test,
// it soon gets impossibly expensive e.g. ~15 mins for a 700 X 832 grayscale bitmap
// A transliteration to something more efficient(e.g.C) is needed for serious work.

#include <vector>
#include <iostream>

using namespace std;
typedef unsigned char byte;

//Concatenation of vectors
vector<byte> conc(const vector<byte> &m, const vector<byte> &n)
{
	vector<byte> ret = m;
	ret.insert(ret.end(), n.begin(), n.end());
	return ret;
}

// bitwise xor on bit - strings
// a, b must be bitsrings of the same, non - zero, length
vector<byte> rowxor(const vector<byte> &a, const vector<byte> &b) {
	if (a.size() != b.size())
		throw exception("string lengths unequal");
	vector<byte> ret(a);

	for (size_t i = 0; i < a.size(); i++) {
		ret[i] ^= b[i];
	}
	return ret;
}

// counts number of 1s in a bit - string
byte norm(const vector<byte> &a) {
	byte cnt = 0;
	for (size_t i = 0; i < a.size(); i++)
		if (a[i] == 1)
			cnt++;
	return cnt;
}

// weighted addition of bit vals at posns 1, 2, 4, 8, 16, 32
// to give a message / grayscale value in range 0 - 63
byte msgval(const vector<byte> &a) {
	return (32 * a[0] + 16 * a[1] + 8 * a[3] + 4 * a[7] + 2 * a[15] + a[31]);
}

void Print(const vector<byte> &a) {
	for (size_t i = 0; i < a.size(); i++)
		printf("%d", a[i]);
}

void PrintN(const vector<byte> &a) {
	Print(a);
	printf("\n");
}

// finds the Hamming distance of testcode from each of the codewords in H5
// referenced via the map array.Returns position of best match i.e.smallest distance
// if "diagnotics" is set to 'false' then default behaviour is to report map position with smallest
// distance.A flag value of 'true' is for diagnostic printout of all distances.
byte codematch(const vector<vector<byte>> &cw, const vector<byte> &map, const vector<byte> &testcode, bool diagnotics) {
	byte pos = 0, bestdist = 32, dist;
	for (int i = 0; i < 64; i++) {
		dist = norm(rowxor(cw[map[i]], testcode));
		if (dist < bestdist) {
			bestdist = dist; pos = i;
		}
		if (diagnotics)
			printf("grayscale = %d, dist =  %d\n", i, dist);
		else if (bestdist == 0)
			break; //leave loop early if match found and flag == 0
	}
	printf("Input vector is ");
	Print(testcode);
	printf("\nBest  match  is ");
	Print(cw[map[pos]]);
	printf(" at  posn / grayscale = \"%d\", dist = %d\n\n\n", pos, bestdist);
	return bestdist;
}

int main() {
	printf("Welcome to the H5 (Reed-Muller) test suite\n");
	// basis vectors for H0
	vector<byte> k0, k1;
	k0 = { 0 }; // Just for symmetry
	k1 = { 1 };

	// basis vectors for H1
	vector<byte> v0, v1, v2;
	v0 = { 0,0 };
	v1 = { 0,1 };
	v2 = { 1,1 };

	// basis vectors for H2
	vector<byte> x0, x1, x2, x3;
	x0 = { 0,0,0,0 };
	x1 = { 0,0,1,1 };
	x2 = { 0,1,0,1 };
	x3 = { 1,1,1,1 };
	//basis vectors for H3
	vector<byte> y0, y1, y2, y3, y4;
	y0 = { 0,0,0,0,0,0,0,0 };
	y1 = { 0,0,0,0,1,1,1,1 };
	y2 = { 0,0,1,1,0,0,1,1 };
	y3 = { 0,1,0,1,0,1,0,1 };
	y4 = { 1,1,1,1,1,1,1,1 };
	// basis vectors for H4
	vector<byte> z0, z1, z2, z3, z4, z5;
	z0 = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	z1 = { 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1 };
	z2 = { 0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1 };
	z3 = { 0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1 };
	z4 = { 0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1 };
	z5 = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
	// Basis vectors for H5.
	// Build these from concatenations of the z basis vectors for H4
	vector<byte> w0, w1, w2, w3, w4, w5, w6;
	w0 = conc(z0, z0); w1 = conc(z0, z5); w2 = conc(z1, z1); w3 = conc(z2, z2);
	w4 = conc(z3, z3); w5 = conc(z4, z4); w6 = conc(z5, z5);
	// H5 codewords will be built up in the array cw from all combinations of the above w[0 - 6].
	// Note that the 6 "message bits" in an H5 codeword(reading from the left and starting at 1)
	// are at bit positions 1, 2, 4, 8, 16, 32. The ordering of codewords in the cw array
	// might seem arbitrary but it owes much to Pascal's Triangle - which makes hand-checking easier -
	// See assignments to "cw" below.
	// The integer array "map".that now follows.points at elements in the cw array and reorders
	// them so as to impose an ascending order of 6 - bit message values i.e.increasing grayscale vals..
	// Hence, map[0] leads to msg bits of "000000", map[31]  to msg bits of "011111" and map[63]
	// delivers a msg of "111111" - and so on.
	//
	vector<byte> map(64);
	map[0 ] = 0 ; map[1 ] = 1 ; map[2 ] = 7 ; map[3 ] = 2 ; map[4 ] = 12; map[5 ] = 22; map[6 ] = 8 ; map[7 ] = 3 ;
	map[8 ] = 16; map[9 ] = 26; map[10] = 42; map[11] = 32; map[12] = 13; map[13] = 23; map[14] = 9 ; map[15] = 4 ;
	map[16] = 19; map[17] = 29; map[18] = 45; map[19] = 35; map[20] = 52; map[21] = 57; map[22] = 48; map[23] = 38;
	map[24] = 17; map[25] = 27; map[26] = 43; map[27] = 33; map[28] = 14; map[29] = 24; map[30] = 10; map[31] = 5 ;
	map[32] = 21; map[33] = 31; map[34] = 47; map[35] = 37; map[36] = 54; map[37] = 59; map[38] = 50; map[39] = 40;
	map[40] = 56; map[41] = 61; map[42] = 63; map[43] = 62; map[44] = 55; map[45] = 60; map[46] = 51; map[47] = 41;
	map[48] = 20; map[49] = 30; map[50] = 46; map[51] = 36; map[52] = 53; map[53] = 58; map[54] = 49; map[55] = 39;
	map[56] = 18; map[57] = 28; map[58] = 44; map[59] = 34; map[60] = 15; map[61] = 25; map[62] = 11; map[63] = 6 ;
	//
	//Building up the H5(Reed - Muller) codewords
	//
	//Single vectors that form the basis set
	vector<vector<byte>> cw(64);
	cw[0] = w0; cw[1] = w1; cw[2] = w2; cw[3] = w3; cw[4] = w4; cw[5] = w5; cw[6] = w6;
	// combinations of two basis vectors
	cw[7] = rowxor(w1, w2);
	cw[8] = rowxor(w1, w3);
	cw[9] = rowxor(w1, w4);
	cw[10] = rowxor(w1, w5);
	cw[11] = rowxor(w1, w6);
	cw[12] = rowxor(w2, w3);
	cw[13] = rowxor(w2, w4);
	cw[14] = rowxor(w2, w5);
	cw[15] = rowxor(w2, w6);
	cw[16] = rowxor(w3, w4);
	cw[17] = rowxor(w3, w5);
	cw[18] = rowxor(w3, w6);
	cw[19] = rowxor(w4, w5);
	cw[20] = rowxor(w4, w6);
	cw[21] = rowxor(w5, w6);
	// combs of 3 basis vectors
	cw[22] = rowxor(cw[7], w3); // triples starting 12
	cw[23] = rowxor(cw[7], w4);
	cw[24] = rowxor(cw[7], w5);
	cw[25] = rowxor(cw[7], w6);
	cw[26] = rowxor(cw[8], w4);// triples starting 13
	cw[27] = rowxor(cw[8], w5);
	cw[28] = rowxor(cw[8], w6);
	cw[29] = rowxor(cw[9], w5);// triples starting 14
	cw[30] = rowxor(cw[9], w6);
	cw[31] = rowxor(cw[10], w6);// 156
	cw[32] = rowxor(cw[12], w4); // triples starting 23
	cw[33] = rowxor(cw[12], w5);
	cw[34] = rowxor(cw[12], w6);
	cw[35] = rowxor(cw[13], w5); // triples starting 24
	cw[36] = rowxor(cw[13], w6);
	cw[37] = rowxor(cw[14], w6); // triples starting 25
	cw[38] = rowxor(cw[16], w5); // triples starting 34
	cw[39] = rowxor(cw[16], w6);
	cw[40] = rowxor(cw[17], w6); //356
	cw[41] = rowxor(cw[19], w6); //456
	// combs of 4 basis vectors
	cw[42] = rowxor(cw[22], w4);// quads starting 123
	cw[43] = rowxor(cw[22], w5);
	cw[44] = rowxor(cw[22], w6);
	cw[45] = rowxor(cw[23], w5);// quads starting 124
	cw[46] = rowxor(cw[23], w6);
	cw[47] = rowxor(cw[24], w6);//  1256
	cw[48] = rowxor(cw[26], w5); // quads starting 134
	cw[49] = rowxor(cw[26], w6);
	cw[50] = rowxor(cw[27], w6); //1356
	cw[51] = rowxor(cw[29], w6); // 1456
	cw[52] = rowxor(cw[32], w5); // quads starting 234
	cw[53] = rowxor(cw[32], w6); //
	cw[54] = rowxor(cw[33], w6); // 2356
	cw[55] = rowxor(cw[35], w6); // 2456
	cw[56] = rowxor(cw[38], w6); // 3456
	// quintets and the final sextet
	cw[57] = rowxor(cw[42], w5); // 12345
	cw[58] = rowxor(cw[42], w6); // 12346
	cw[59] = rowxor(cw[43], w6); // 12356
	cw[60] = rowxor(cw[45], w6); // 12456
	cw[61] = rowxor(cw[48], w6); // 13456
	cw[62] = rowxor(cw[52], w6); // 23456
	cw[63] = rowxor(cw[57], w6); //123456

	// print out codewords in "Pascal Triangle" order
	//for (int i = 0; i < cw.size(); i++)
	//	PrintN(cw[i]);

	printf("\nHere are the H5 codewords in ascending order of message bit values \n\n");
	for (size_t i = 0; i < cw.size(); i++)
		PrintN(cw[map[i]]);

	// And here's a test printout to verify that message-bit values are OK and range from 0-63
	printf("\nHere are the 64 gray values contained in msg bits at posns 1, 2, 4, 8, 16, 32\n");
	for (size_t i = 0; i < cw.size(); i++)
		printf("%d\n", msgval(cw[map[i]]));

	// Declare a few test  messages
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

	// Test print of the codeword norms
	//printf("\nHere are the norms in order\n\n");
	//for (int i = 0; i < cw.size(); i++)
	//	printf("%d ", norm(cw[map[i]]));

	printf("\nEnd of run \n");
	return 0;
}