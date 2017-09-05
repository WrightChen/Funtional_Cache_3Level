#ifndef _SET_H_
#define _SET_H_

#include <map>
#include "Way.h"

#define L1_OFFSET           ((1UL<<12)-1)
#define L2_OFFSET           ((1UL<<15)-1)
#define L3_OFFSET           ((1UL<<16)-1)
#define L1_MASK             (~((1UL<<12)-1))

struct Set
{
    public:
        std::map<uint64_t, Way> way;
        uint64_t lrurec[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        bool lru[7] = {0, 0, 0, 0, 0, 0, 0};

};

struct Set_l2
{
    public:
        std::map<uint64_t, Way> way;
        std::map<uint64_t, Shadow> shadow;
        uint64_t lrurec[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        bool lru[7] = {0, 0, 0, 0, 0, 0, 0};

};

struct Set_l3
{
    public:
        std::map<uint64_t, Way_l3> way;
        uint64_t lrurec[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        bool lru[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

};


#endif