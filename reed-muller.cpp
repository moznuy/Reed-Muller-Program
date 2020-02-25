#include "reed-muller.h"
#include <stdexcept>
#include <algorithm>
#include <set>


using namespace std;

//Concatenation of one vector to self n times
void operator *=(vector<bool> &a, BYTE n) {
    size_t oldSize;
    for (BYTE i = 1; i < n; i++) {
        oldSize = a.size();
        a.reserve(2 * oldSize);
        copy_n(a.begin(), oldSize, back_inserter(a));
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
BYTE norm(const vector<bool> &a) {
    BYTE cnt = 0;
    for (bool i : a)
        if (i == 1)
            cnt++;
    return cnt;
}

// weighted addition of bit vals at posns 1, 2, 4, 8, 16, 32 ...
// to give a message / grayscale value in range 0 - 63[for H[5]] ...
BYTE msgval(const vector<bool> &a) {
    BYTE sum = 0;
    for (size_t i = 0, j = 0, k = a.size(); i < a.size(); j++, k >>= 1u, i = (1u << j) - 1)
        sum += (BYTE)k * (int)a[i]; //(32 * a[0] + 16 * a[1] + 8 * a[3] + 4 * a[7] + 2 * a[15] + a[31]) for H[5]
    return sum;
}

// Fills set with indexes of important bits (info bits)
void FillImportantBits(set<BYTE> &importantBits, BYTE size) {
    for (BYTE i = 0, j = 0; i < size; i = (1u << ++j) - 1)
        importantBits.insert(i);
}

// Marks position of important bits with '|' character
void PrintImportantBits(const set<BYTE> &importantBits, BYTE size) {
    if (size > 128)
        return; //for economy space and time
    for (BYTE i = 0; i < size; i++) {
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
        for (bool i : a)
            printf("%d", (int)i);

    else //for economy space and time
    {
        BYTE sum = 0;
        printf("0x");
        for (size_t i = 0; i < a.size() / 4; i++) {
            sum = (a[a.size() - (i * 4 + 0) - 1] << 0u)
                  + (a[a.size() - (i * 4 + 1) - 1] << 1u)
                  + (a[a.size() - (i * 4 + 2) - 1] << 2u)
                  + (a[a.size() - (i * 4 + 3) - 1] << 3u);
            printf("%X", sum);
        }
    }
}

// Finds the smallest Hamming distance of testcode from all of the codewords in H[N]
BYTE codematch(const vector<vector<bool> > &cw, const vector<BYTE> &map, const vector<bool> &testcode, bool diagnotics) {
    BYTE pos = 0, bestdist = (BYTE)testcode.size(), dist;
    for (size_t i = 0; i < cw.size(); i++) {
        dist = norm(cw[map[i]] ^ testcode);
        if (dist < bestdist) {
            bestdist = dist; pos = (BYTE)i;
        }
        if (diagnotics)
            printf("grayscale = %lu, dist =  %d\n", i, dist);
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
vector<vector<bool> > getAllH(BYTE deep) {
    vector<vector<bool> > ret;
    if (deep == 0) {
        ret.push_back({ false });  // 0
        ret.push_back({ true });   // 1
        return ret;
    }

    ret = getAllH(deep - 1);

    // concat
    for (auto & i : ret)
        i *= 2;

    // row of zeros and ones like in the video
    vector<bool> rowOf0A1;
    for (size_t i = 0; i < (1u << (deep - 1)); i++)
        rowOf0A1.push_back(false);  // 0
    for (size_t i = 0; i < (1u << (deep - 1)); i++)
        rowOf0A1.push_back(true);   // 1
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
pair<BYTE, BYTE> generateTreeMasks(vector<pair<BYTE, BYTE> > &masks, BYTE len, BYTE size, BYTE parrent, BYTE start) {
    BYTE index, count = 0;
    if (len == 1) {
        index = (BYTE)masks.size();
        count = size - start;
        for (BYTE i = start; i < size; i++)
            masks.emplace_back(parrent, i);
        return make_pair(count, index); // return number of pairs and index of the first one
    }
    pair<BYTE, BYTE> prev = generateTreeMasks(masks, len - 1, size, parrent, start); //get all masks of previous length
    index = (BYTE)masks.size();
    for (BYTE i = 0; i < prev.first; i++) {
        count += generateTreeMasks(masks, 1, size, prev.second + i, masks[prev.second + i].second + 1).first;
    }
    return make_pair(count, index); // return number of pairs and index of the first one
}

// Frame for generateTreeMasks()
vector<pair<BYTE, BYTE> > generateAllMasks(BYTE size) {
    vector<pair<BYTE, BYTE> > ret;
    ret.emplace_back(-1, -1); // represent mask 000..0
    generateTreeMasks(ret, size, size, -1, 0);
    return ret;
}

// Get code word recursivly if not found previously
vector<bool> getCodeWord(const vector<pair<BYTE, BYTE> > &masks, BYTE index, const vector<vector<bool> > &h, vector<vector<bool> > &res) {
    if (!res[index].empty())
        return res[index];
    if (masks[index].first == -1)
        return h[masks[index].second + 1];

    return getCodeWord(masks, masks[index].first, h, res) ^ h[masks[index].second + 1]; //bitwise xor
}

// Maps H[N] to all code words
vector<vector<bool> > mapCodewordsUsingMasks(const vector<pair<BYTE, BYTE> > &masks, const vector<vector<bool> > &h) {
    vector<vector<bool> > ret(masks.size());
    for (size_t i = 0; i < masks.size(); i++)
        ret[i] = getCodeWord(masks, i, h, ret);
    //ret.insert(ret.begin(), h[0]);
    return ret;
}

// Determines which number corresponds to which word
vector<BYTE> GenetateMapping(const vector<vector<bool> > &cw) {
    vector<BYTE> ret(cw.size());
    for (size_t i = 0; i < cw.size(); i++)
        ret[msgval(cw[i])] = i;
    return ret;
}
