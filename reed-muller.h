#ifndef REED_MULLER_REED_MULLER_H
#define REED_MULLER_REED_MULLER_H

#include <vector>
#include <set>

//First it was unsigned char but for n > 7 you need more space
typedef unsigned int BYTE;


BYTE msgval(const std::vector<bool> &a);
void FillImportantBits(std::set<BYTE> &importantBits, BYTE size);
void PrintImportantBits(const std::set<BYTE> &importantBits, BYTE size);
void Print(const std::vector<bool> &a);
BYTE codematch(
        const std::vector<std::vector<bool> > &cw,
        const std::vector<BYTE> &map,
        const std::vector<bool> &testcode,
        bool diagnotics);
std::vector<std::vector<bool> > getAllH(BYTE deep);
std::pair<BYTE, BYTE> generateTreeMasks(
        std::vector<std::pair<BYTE, BYTE> > &masks,
        BYTE len,
        BYTE size,
        BYTE parrent,
        BYTE start);
std::vector<std::pair<BYTE, BYTE> > generateAllMasks(BYTE size);
std::vector<bool> getCodeWord(
        const std::vector<std::pair<BYTE, BYTE> > &masks,
        BYTE index,
        const std::vector<std::vector<bool> > &h,
        std::vector<std::vector<bool> > &res);
std::vector<std::vector<bool> > mapCodewordsUsingMasks(
        const std::vector<std::pair<BYTE, BYTE> > &masks,
        const std::vector<std::vector<bool> > &h);
std::vector<BYTE> GenetateMapping(const std::vector<std::vector<bool> > &cw);

#endif //REED_MULLER_REED_MULLER_H
