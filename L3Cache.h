#ifndef _L3CACHE_H_
#define _L3CACHE_H_

#define L3_SET_NUM 8192

#include <iostream>
#include <array>
#include "Set.h"
#include "Lru.h"

using namespace std;

class L1Cache;
class L2Cache;

class L3Cache
{
    public:
        L3Cache(unsigned int id);
        void Load(uint64_t addr, unsigned int l1_id, unsigned int l2_id);
        void LoadInv(uint64_t addr, unsigned int l1_id, unsigned int l2_id, bool l2_hit);

        void UpdateL3Record(uint64_t addr, uint8_t core_num, CacheMESI r_mesi);
        void UpdateL3(uint64_t addr, CacheMESI l3_mesi, uint8_t core_num, CacheMESI r_mesi);
        
        void SetL1Ptr(L1Cache* l1_ptr, unsigned int l1_id){
            m_l1_ptr[l1_id] = l1_ptr;
        }
        void SetL2Ptr(L2Cache* l2_ptr, unsigned int l2_id){
            m_l2_ptr[l2_id] = l2_ptr;
        }

        void Statistics(){

            if(m_load_counter != 0)
            cout << "[L3][Statistics]" << endl 
                 << "[Load][Total] " << dec << m_load_counter << endl
                 << "[Load][Hit] " << dec << m_load_hit << endl
                 << "[Load][Hit Rate] " << double(m_load_hit) / double(m_load_counter) * 100 << "%" << endl
                 << endl;
                
            if(m_loadinv_counter != 0)
            cout << "[L3][Statistics]" << endl 
                 << "[LoadInv][Total] " << dec << m_loadinv_counter << endl
                 << "[LoadInv][Hit] " << dec << m_loadinv_hit << endl
                 << "[LoadInv][Hit Rate] " << double(m_loadinv_hit) / double(m_loadinv_counter) * 100 << "%" << endl
                 << endl;
        }


    private:
		unsigned int						m_id;
        unsigned int						m_l3_size;
		unsigned int						m_way_num;
		unsigned int						m_line_size;
		unsigned int						m_set_num;

        unsigned int                        m_load_counter;
        unsigned int                        m_load_hit;
        unsigned int                        m_loadinv_counter;
        unsigned int                        m_loadinv_hit;

        array<Set_l3, L3_SET_NUM>           m_l3;

        Lru                                 m_lru;
        L1Cache*                            m_l1_ptr[16];
        L2Cache*                            m_l2_ptr[8];

};

#endif