#ifndef _L2CACHE_H_
#define _L2CACHE_H_

#define L2_SET_NUM 512

#include <iostream>
#include <array>
#include "Set.h"
#include "Lru.h"

#include "L3Cache.h"

using namespace std;

class L1Cache;

class L2Cache
{
    public:
        L2Cache(unsigned int id);
        void Load(uint64_t addr, unsigned int l1_id);
        void LoadInv(uint64_t addr, unsigned int l1_id, bool l1_hit);

        bool Lookup(uint64_t addr);

        CacheMESI UpdateL2Shadow(uint64_t addr, uint8_t l1_id, CacheMESI l1_mesi);
        void UpdateL2(uint64_t addr, uint8_t l1_id, CacheMESI l1_mesi, CacheMESI l2_mesi);

        bool QueryL1L2(uint64_t addr, CacheMESI mesi);

        void SetL1Ptr(L1Cache* l1i_ptr, L1Cache* l1d_ptr){
            m_l1_ptr[0] = l1i_ptr;
            m_l1_ptr[1] = l1d_ptr;
        }
        void SetL3Ptr(L3Cache* l3_ptr){
            m_l3_ptr = l3_ptr;
        }

        void Statistics(){
            
            if(m_load_counter != 0)
            cout << "[L2_" << m_id << "][Statistics]" << endl 
                 << "[Load][Total] " << dec << m_load_counter << endl
                 << "[Load][Hit] " << dec << m_load_hit << endl
                 << "[Load][Hit Rate] " << double(m_load_hit) / double(m_load_counter) * 100 << "%" << endl
                 << endl;
            
            if(m_loadinv_counter != 0)
            cout << "[L2_" << m_id << "][Statistics]" << endl 
                 << "[LoadInv][Total] " << dec << m_loadinv_counter << endl
                 << "[LoadInv][Hit] " << dec << m_loadinv_hit << endl
                 << "[LoadInv][Hit Rate] " << double(m_loadinv_hit) / double(m_loadinv_counter) * 100 << "%" << endl
                 << endl;
        }

    private:
		unsigned int						m_id;
        unsigned int						m_l2_size;
		unsigned int						m_way_num;
		unsigned int						m_line_size;
		unsigned int						m_set_num;

        unsigned int                        m_load_counter;
        unsigned int                        m_load_hit;
        unsigned int                        m_loadinv_counter;
        unsigned int                        m_loadinv_hit;

        array<Set_l2, L2_SET_NUM>           m_l2;

        Lru                                 m_lru;
        L1Cache*                            m_l1_ptr[2];
        L3Cache*                            m_l3_ptr;

};

#endif