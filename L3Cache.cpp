#include "L1Cache.h"
#include "L3Cache.h"

L3Cache::L3Cache(unsigned int id)
: m_id(id)
, m_l3_size(8*1024*1024)
, m_way_num(16)
, m_line_size(64)
, m_set_num(8192)
, m_load_counter(0)
, m_load_hit(0)
, m_loadinv_counter(0)
, m_loadinv_hit(0)
{
    
}


/*----------------------------------------------------------------------------------------------------------------------------------------------*/


void 
L3Cache::Load(uint64_t addr, unsigned int l1_id, unsigned int l2_id)
{
    bool t_share = false, t_modify = false;
    int t_num, t_loc;
    uint8_t h0, h1, h2;
    uint64_t t_tag, t_addr, t_evict;
    Way_l3 t_way;

    ++m_load_counter;

    cout << "[L3][Load] " << "Address = " << addr << endl;

    h0 = 0x1 & ((addr>>36)^(addr>>35)^(addr>>33)^(addr>>32)^(addr>>30)^(addr>>28)^(addr>>27)^(addr>>26)^(addr>>25)^(addr>>24)^(addr>>22)^(addr>>20)^(addr>>18)^(addr>>17)^(addr>>16)^(addr>>14)^(addr>>12)^(addr>>10)^(addr>>6));
	h1 = 0x1 & ((addr>>37)^(addr>>35)^(addr>>34)^(addr>>33)^(addr>>31)^(addr>>29)^(addr>>28)^(addr>>26)^(addr>>24)^(addr>>23)^(addr>>22)^(addr>>21)^(addr>>20)^(addr>>19)^(addr>>17)^(addr>>15)^(addr>>13)^(addr>>11)^(addr>>7));
	h2 = 0x1 & ((addr>>37)^(addr>>36)^(addr>>35)^(addr>>34)^(addr>>31)^(addr>>30)^(addr>>27)^(addr>>26)^(addr>>23)^(addr>>22)^(addr>>19)^(addr>>16)^(addr>>13)^(addr>>12)^(addr>>8));

    t_tag = addr >> 16;
    t_num = ((addr & L3_OFFSET) >> 6) + 1024 * (4*h2+2*h1+h0);

    cout << "[L3][Load] " << "Set_num = " << t_num << endl;

    if(m_l3[t_num].way.find(t_tag) == m_l3[t_num].way.end()){
        // L3 Cache Miss.
        if(m_l3[t_num].way.size() < m_way_num){
            //--------------------------------------------- Have free entry. ---------------------------------------------
            t_loc = m_lru.getfree_16(m_l3[t_num].lrurec);
            m_lru.updlru_16(m_l3[t_num].lru, t_loc);
            t_way.mesi = E;
            t_way.loc = t_loc;
            t_way.record[l2_id] = E;
            m_l3[t_num].way.insert({t_tag, t_way});
            m_l3[t_num].lrurec[t_loc] = addr;

            cout << "[L3][Load][Located] " << t_loc << endl;

        }
        else{
            //--------------------------------------------- No free entry, need evict. ---------------------------------------------
            t_loc = m_lru.getlru_16(m_l3[t_num].lru);
            t_addr = m_l3[t_num].lrurec[t_loc];
            t_evict = t_addr >> 16;

            for(unsigned int i = 0; i < 8; ++i)
            {
                if(m_l3[t_num].way[t_evict].record[i] != I){
                    m_l2_ptr[i]->QueryL1L2(t_addr, I);
                }
            }

            m_l3[t_num].way.erase(t_evict);
            t_way.mesi = E;
            t_way.loc = t_loc;
            t_way.record[l2_id] = E;
            m_l3[t_num].way.insert({t_tag, t_way});
            m_l3[t_num].lrurec[t_loc] = addr;

            cout << "[L3][Load][Located] " << t_loc << endl;
        }
    }
    else{
        //------------------------------------------------ L3 Cache Hit. ------------------------------------------------
        ++m_load_hit;
        for(unsigned int i = 0; i < 8; ++i)
        {
            if(i != l2_id){
                if(m_l3[t_num].way[t_tag].record[i] != I){
                    if(m_l2_ptr[i]->QueryL1L2(addr, S))
                        t_modify = true;
                    t_share= true;
                }
            }
        }
        
        if(t_share){
            m_l1_ptr[l1_id]->UpdateL1(addr, S);
            m_l2_ptr[l2_id]->UpdateL2(addr, l1_id%2, S, S);
            m_l3[t_num].way[t_tag].record[l2_id] = S;
        }
        else{
            m_l3[t_num].way[t_tag].record[l2_id] = E;
        }

        if(t_modify)
            m_l3[t_num].way[t_tag].mesi = M;

        t_loc = m_l3[t_num].way[t_tag].loc;
        m_lru.updlru_16(m_l3[t_num].lru, t_loc);

        cout << "[L3][Load][Located] " << t_loc << endl;
    }
        

    cout << "[L3][Load][Size] " << m_l3[t_num].way.size() << endl;
}


/*----------------------------------------------------------------------------------------------------------------------------------------------*/


void 
L3Cache::LoadInv(uint64_t addr, unsigned int l1_id, unsigned int l2_id, bool l2_hit)
{
    bool t_modify = false;
    int t_num, t_loc;
    uint8_t h0, h1, h2;
    uint64_t t_tag, t_addr, t_evict;
    Way_l3 t_way;

    if(!l2_hit)
        ++m_loadinv_counter;

    h0 = 0x1 & ((addr>>36)^(addr>>35)^(addr>>33)^(addr>>32)^(addr>>30)^(addr>>28)^(addr>>27)^(addr>>26)^(addr>>25)^(addr>>24)^(addr>>22)^(addr>>20)^(addr>>18)^(addr>>17)^(addr>>16)^(addr>>14)^(addr>>12)^(addr>>10)^(addr>>6));
	h1 = 0x1 & ((addr>>37)^(addr>>35)^(addr>>34)^(addr>>33)^(addr>>31)^(addr>>29)^(addr>>28)^(addr>>26)^(addr>>24)^(addr>>23)^(addr>>22)^(addr>>21)^(addr>>20)^(addr>>19)^(addr>>17)^(addr>>15)^(addr>>13)^(addr>>11)^(addr>>7));
	h2 = 0x1 & ((addr>>37)^(addr>>36)^(addr>>35)^(addr>>34)^(addr>>31)^(addr>>30)^(addr>>27)^(addr>>26)^(addr>>23)^(addr>>22)^(addr>>19)^(addr>>16)^(addr>>13)^(addr>>12)^(addr>>8));

    cout << "[L3][LoadInv] " << "Address = " << addr << endl;

    t_tag = addr >> 16;
    t_num = ((addr & L3_OFFSET) >> 6) + 1024 * (4*h2+2*h1+h0);

    cout << "[L3][LoadInv] " << "Set_num = " << t_num << endl;

    if(m_l3[t_num].way.find(t_tag) == m_l3[t_num].way.end()){
        // L3 Cache Miss.
        if(m_l3[t_num].way.size() < m_way_num){
            //--------------------------------------------- Have free entry. ---------------------------------------------
            t_loc = m_lru.getfree_16(m_l3[t_num].lrurec);
            m_lru.updlru_16(m_l3[t_num].lru, t_loc);
            t_way.mesi = E;
            t_way.loc = t_loc;
            t_way.record[l2_id] = E;
            m_l3[t_num].way.insert({t_tag, t_way});
            m_l3[t_num].lrurec[t_loc] = addr;
            m_l3[t_num].way[t_tag].record[l2_id] = E;

            cout << "[L3][LoadInv][Located] " << t_loc << endl;
        }
        else{
            //--------------------------------------------- No free entry, need evict. ---------------------------------------------
            t_loc = m_lru.getlru_16(m_l3[t_num].lru);
            t_addr = m_l3[t_num].lrurec[t_loc];
            t_evict = t_addr >> 16;

            for(unsigned int i = 0; i < 8; ++i)
            {
                if(m_l3[t_num].way[t_evict].record[i] != I){
                    m_l2_ptr[i]->QueryL1L2(t_addr, I);
                }
            }

            m_l3[t_num].way.erase(t_evict);
            t_way.mesi = E;
            t_way.loc = t_loc;
            t_way.record[l2_id] = E;
            m_l3[t_num].way.insert({t_tag, t_way});
            m_l3[t_num].lrurec[t_loc] = addr;

            cout << "[L3][LoadInv][Located] " << t_loc << endl;
        }
    }
    else{
        //------------------------------------------------ L3 Cache Hit. ------------------------------------------------
        if(!l2_hit)
            ++m_loadinv_hit;
        for(unsigned int i = 0; i < 8; ++i)
        {
            if(i != l2_id){
                if(m_l3[t_num].way[t_tag].record[i] != I){
                    if(m_l2_ptr[i]->QueryL1L2(addr, I))
                        t_modify = true;
                    m_l3[t_num].way[t_tag].record[i] = I;
                }
            }
        }

        if(t_modify)
            m_l3[t_num].way[t_tag].mesi = M;

        m_l3[t_num].way[t_tag].record[l2_id] = E;
        t_loc = m_l3[t_num].way[t_tag].loc;
        m_lru.updlru_16(m_l3[t_num].lru, t_loc);

        cout << "[L3][LoadInv][Located] " << t_loc << endl;
    }
        

    cout << "[L3][LoadInv][Size] " << m_l3[t_num].way.size() << endl;
}



void 
L3Cache::UpdateL3Record(uint64_t addr, uint8_t core_num, CacheMESI r_mesi)
{
    uint8_t h0, h1, h2;
    unsigned int t_num;
    uint64_t t_tag;

    h0 = 0x1 & ((addr>>36)^(addr>>35)^(addr>>33)^(addr>>32)^(addr>>30)^(addr>>28)^(addr>>27)^(addr>>26)^(addr>>25)^(addr>>24)^(addr>>22)^(addr>>20)^(addr>>18)^(addr>>17)^(addr>>16)^(addr>>14)^(addr>>12)^(addr>>10)^(addr>>6));
	h1 = 0x1 & ((addr>>37)^(addr>>35)^(addr>>34)^(addr>>33)^(addr>>31)^(addr>>29)^(addr>>28)^(addr>>26)^(addr>>24)^(addr>>23)^(addr>>22)^(addr>>21)^(addr>>20)^(addr>>19)^(addr>>17)^(addr>>15)^(addr>>13)^(addr>>11)^(addr>>7));
	h2 = 0x1 & ((addr>>37)^(addr>>36)^(addr>>35)^(addr>>34)^(addr>>31)^(addr>>30)^(addr>>27)^(addr>>26)^(addr>>23)^(addr>>22)^(addr>>19)^(addr>>16)^(addr>>13)^(addr>>12)^(addr>>8));

    t_tag = addr >> 16;
    t_num = ((addr & L3_OFFSET) >> 6) + 1024 * (4*h2+2*h1+h0);

    m_l3[t_num].way[t_tag].record[core_num] = r_mesi;
}




void 
L3Cache::UpdateL3(uint64_t addr, CacheMESI mesi, uint8_t core_num, CacheMESI r_mesi)
{
    uint8_t h0, h1, h2;
    unsigned int t_num;
    uint64_t t_tag;

    h0 = 0x1 & ((addr>>36)^(addr>>35)^(addr>>33)^(addr>>32)^(addr>>30)^(addr>>28)^(addr>>27)^(addr>>26)^(addr>>25)^(addr>>24)^(addr>>22)^(addr>>20)^(addr>>18)^(addr>>17)^(addr>>16)^(addr>>14)^(addr>>12)^(addr>>10)^(addr>>6));
	h1 = 0x1 & ((addr>>37)^(addr>>35)^(addr>>34)^(addr>>33)^(addr>>31)^(addr>>29)^(addr>>28)^(addr>>26)^(addr>>24)^(addr>>23)^(addr>>22)^(addr>>21)^(addr>>20)^(addr>>19)^(addr>>17)^(addr>>15)^(addr>>13)^(addr>>11)^(addr>>7));
	h2 = 0x1 & ((addr>>37)^(addr>>36)^(addr>>35)^(addr>>34)^(addr>>31)^(addr>>30)^(addr>>27)^(addr>>26)^(addr>>23)^(addr>>22)^(addr>>19)^(addr>>16)^(addr>>13)^(addr>>12)^(addr>>8));

    t_tag = addr >> 16;
    t_num = ((addr & L3_OFFSET) >> 6) + 1024 * (4*h2+2*h1+h0);

    if(mesi != I)
        m_l3[t_num].way[t_tag].mesi = mesi;
    else{
        m_l3[t_num].lrurec[m_l3[t_num].way[t_tag].loc] = 0;
        m_l3[t_num].way.erase(t_tag);
    }
        
    m_l3[t_num].way[t_tag].record[core_num] = r_mesi;
}