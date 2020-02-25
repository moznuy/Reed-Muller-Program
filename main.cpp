#define _CRT_SECURE_NO_WARNINGS // for Visual Studio
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <set>
#include <random>

#include "reed-muller.h"

using namespace std;


int main(int args, char **argv) {
    random_device rd;
    mt19937 gen(rd());
    BYTE n, tests;

    if (args < 3) {
        cout << "Welcome to the H[n] (Reed-Muller) test suite\n\
Usage: read-muller n ntests\n\
n: H[n]\n\
ntests: number of tests\n\
You should probably redirect output to file\n\
Note: For n = 15 program allocated 280MB in memory, genereted file 525MB big! and worked about 2 minutes!\n\
Note: Size and number of code words is proportional to 2^N so be carefull!" << endl;
        return 1;
    }

#ifdef _DEBUG
    n = 5;
#else
    stringstream n_stream(argv[1]);
    n_stream >> n;
#endif

#ifdef _DEBUG
    tests = 5;
#else
    stringstream tests_stream(argv[2]);
    tests_stream >> tests;
#endif

    vector<vector<bool> > H = getAllH(n);
    vector<pair<BYTE, BYTE> > masksTree = generateAllMasks((BYTE)H.size() - 1);
    vector<vector<bool> > cw = mapCodewordsUsingMasks(masksTree, H);
    vector<BYTE> map = GenetateMapping(cw);

    printf("\nHere are the H[%d] codewords in ascending order of message bit values \n| - corresponds to information bits\n\n", n);
    set<BYTE> importantBits;
    FillImportantBits(importantBits, (BYTE)cw[0].size());
    PrintImportantBits(importantBits, (BYTE)cw[0].size());
    for (size_t i = 0; i < cw.size(); i++) {
        Print(cw[map[i]]);
        printf(" - %lu\n", i);
    }

    printf("\nTest. %lu values should be in the ascending order\n", cw.size());
    int sq = (int)sqrt(cw.size());
    for (size_t i = 0; i < cw.size(); i++) {
        printf("%d ", msgval(cw[map[i]]));
        if ((i + 1) % sq == 0)
            printf("\n");
    }

    printf("\n\n");
    vector<bool> random;
    BYTE k, l, z;
    uniform_int_distribution<> codeword_distribution(0, cw.size() - 1);
    uniform_int_distribution<> mistake_count_distribution(0, cw.size() - 1);

    for (BYTE t = 0; t < tests; t++) {
        // one of code words
        k = codeword_distribution(gen);
        // small number of mistakes
        l = mistake_count_distribution(gen);
        random = cw[k];
        for (BYTE i = 0; i < l; i++) {
            z = rand() % random.size();
            random[z] = !random[z];
        }
        codematch(cw, map, random, false);
    }


#ifdef _OldTestingFromVideo
    vector<BYTE> test63    = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
    vector<BYTE> test15    = { 0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1 }; //roughly at pos15
    vector<BYTE> test48    = { 1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0 };
    vector<BYTE> newtest48 = { 1,0,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0 }; // roughly at 48 but 5 bits damaged
    vector<BYTE> test0     = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; //zero vector undamaged
    vector<BYTE> newtest0  = { 0,0,1,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1 }; //zero vector but damaged in 7 places
    vector<BYTE> random    = { 1,0,1,1,1,0,0,1,0,1,0,0,0,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0 }; // random string of 0s and 1s

    //for (size_t i = 0; i < cw.size(); i++)
    //	printf("i = %d dist = %d\n", i, norm(rowxor(cw[map[i]], newtest48)));

    BYTE x;
    x = codematch(cw, map, test15, false);
    x = codematch(cw, map, newtest48, false);
    printf("\nShow every stage of brute-force testing for test vector distance 8 from grayshade 44\n");
    x = codematch(cw, map, random, true);
#endif
    return 0;
}