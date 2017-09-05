#include "L1Cache.h"
#include "L2Cache.h"


L2Cache::L2Cache(unsigned int id)
: m_id(id)
, m_l2_size(256*1024)
, m_way_num(8)
, m_line_size(64)
, m_set_num(512)
, m_load_counter(0)
, m_load_hit(0)
, m_loadinv_counter(0)
, m_loadinv_hit(0)
{
    
}

/*----------------------------------------------------------------------------------------------------------------------------------------------*/

void 
L2Cache::Load(uint64_t addr, unsigned int l1_id)
{
    unsigned int t_num, t_loc;
    uint64_t t_tag, t_addr, t_evict;
    Way t_way;

    ++m_load_counter;

    cout << "[L2][Load] " << "Address = " << addr << endl;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;
    
    cout << "[L2][Load] " << "Set_num = " << t_num << endl;

    if(m_l2[t_num].way.find(t_tag) == m_l2[t_num].way.end()){
        // L2 Cache Miss
        if(m_l2[t_num].way.size() < m_way_num){
            //--------------------------------------------- Have free entry ---------------------------------------------
            t_loc = m_lru.getfree(m_l2[t_num].lrurec);
            m_lru.updlru(m_l2[t_num].lru, t_loc);
            t_way.mesi = E;
            t_way.loc = t_loc;
            m_l2[t_num].way.insert({t_tag, t_way});
            m_l2[t_num].lrurec[t_loc] = addr;
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = E;

            m_l3_ptr->Load(addr, m_id*2 + l1_id, m_id);

            cout << "[L2][Load][Located] " << t_loc << endl;
        }
        else{
            //--------------------------------------------- No free entry, need evict ---------------------------------------------
            t_loc = m_lru.getlru(m_l2[t_num].lru);
            t_addr = m_l2[t_num].lrurec[t_loc];
            t_evict = t_addr >> 15;

            if(m_l2[t_num].way[t_evict].mesi == M){
                //Remove from L1I/L1D.
                if(m_l2[t_num].shadow.find(t_evict) != m_l2[t_num].shadow.end()){
                    if(m_l2[t_num].shadow[t_evict].mesi[0] != I){
                        m_l1_ptr[0]->UpdateL1(addr, I);
                    }
                    if(m_l2[t_num].shadow[t_evict].mesi[1] != I){
                        m_l1_ptr[1]->UpdateL1(addr, I);
                    }
                    m_l2[t_num].shadow.erase(t_evict);
                }
                m_l3_ptr->UpdateL3(t_addr, M, m_id, I);
            }
            else{
                if(m_l2[t_num].shadow.find(t_evict) == m_l2[t_num].shadow.end())
                    m_l3_ptr->UpdateL3Record(t_addr, m_id, I);
            }

            m_l2[t_num].lrurec[m_l2[t_num].way[t_evict].loc] = 0;
            m_l2[t_num].way.erase(t_evict);
            t_way.mesi = E;
            t_way.loc = t_loc;
            m_l2[t_num].way.insert({t_tag, t_way});
            m_l2[t_num].lrurec[t_loc] = addr;
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = E;

            m_l3_ptr->Load(addr, m_id*2 + l1_id, m_id);

            cout << "[L2][Load][Located] " << t_loc << endl;
        }

        //Exist in other L1?
        if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] != I){
            if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] == M)
                m_l2[t_num].way[t_tag].mesi = M;
            m_l1_ptr[l1_id]->UpdateL1(addr, S);
            m_l1_ptr[!l1_id]->UpdateL1(addr, S);
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = S;
            m_l2[t_num].shadow[t_tag].mesi[!l1_id] = S;
        }

    }

    else{
        //------------------------------------------------ L2 Cache Hit. ------------------------------------------------
        ++m_load_hit;
        if(m_l2[t_num].way[t_tag].mesi == S){
            m_l1_ptr[l1_id]->UpdateL1(addr, S);
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = S;
        }
        else{
            if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] == I)
                m_l2[t_num].shadow[t_tag].mesi[l1_id] = E;
            else{
                if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] == M)
                    m_l2[t_num].way[t_tag].mesi = M;
                m_l1_ptr[l1_id]->UpdateL1(addr, S);
                m_l1_ptr[!l1_id]->UpdateL1(addr, S);
                m_l2[t_num].shadow[t_tag].mesi[l1_id] = S;
                m_l2[t_num].shadow[t_tag].mesi[!l1_id] = S;
            }
        }
            
        t_loc = m_l2[t_num].way[t_tag].loc;
        m_lru.updlru(m_l2[t_num].lru, t_loc);

        cout << "[L2][Load][Located] " << t_loc << endl;
    }
        

    cout << "[L2][Load][Size] " << m_l2[t_num].way.size() << endl;
}

/*----------------------------------------------------------------------------------------------------------------------------------------------*/

void 
L2Cache::LoadInv(uint64_t addr, unsigned int l1_id, bool l1_hit)
{
    unsigned int t_num, t_loc;
    uint64_t t_tag, t_addr, t_evict;
    Way t_way;

    if(!l1_hit)
        ++m_loadinv_counter;

    cout << "[L2][LoadInv] " << "Address = " << addr << endl;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;
    
    cout << "[L2][LoadInv] " << "Set_num = " << t_num << endl;

    if(m_l2[t_num].way.find(t_tag) == m_l2[t_num].way.end()){
        // L2 Cache Miss

        //Exist in other L1? Kill it.
        if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] != I){
            m_l1_ptr[!l1_id]->UpdateL1(addr, I);
            m_l2[t_num].shadow[t_tag].mesi[!l1_id] = I;
        }

        if(m_l2[t_num].way.size() < m_way_num){
            //--------------------------------------------- Have free entry ---------------------------------------------
            t_loc = m_lru.getfree(m_l2[t_num].lrurec);
            m_lru.updlru(m_l2[t_num].lru, t_loc);
            t_way.mesi = E;
            t_way.loc = t_loc;
            m_l2[t_num].way.insert({t_tag, t_way});
            m_l2[t_num].lrurec[t_loc] = addr;
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = M;

            m_l3_ptr->LoadInv(addr, m_id*2 + l1_id, m_id, false);

            cout << "[L2][LoadInv][Located] " << t_loc << endl;
        }
        else{
            //--------------------------------------------- No free entry, need evict ---------------------------------------------
            t_loc = m_lru.getlru(m_l2[t_num].lru);
            t_addr = m_l2[t_num].lrurec[t_loc];
            t_evict = t_addr >> 15;

            if(m_l2[t_num].way[t_evict].mesi == M){
                //Remove from L1I/L1D.
                if(m_l2[t_num].shadow.find(t_evict) != m_l2[t_num].shadow.end()){
                    if(m_l2[t_num].shadow[t_evict].mesi[0] != I){
                        m_l1_ptr[0]->UpdateL1(addr, I);
                    }
                    if(m_l2[t_num].shadow[t_evict].mesi[1] != I){
                        m_l1_ptr[1]->UpdateL1(addr, I);
                    }
                    m_l2[t_num].shadow.erase(t_evict);
                }
                m_l3_ptr->UpdateL3(t_addr, M, m_id, I);
            }
            else{
                if(m_l2[t_num].shadow.find(t_evict) == m_l2[t_num].shadow.end())
                    m_l3_ptr->UpdateL3Record(t_addr, m_id, I);
            }

            m_l2[t_num].lrurec[m_l2[t_num].way[t_evict].loc] = 0;
            m_l2[t_num].way.erase(t_evict);
            t_way.mesi = E;
            t_way.loc = t_loc;
            m_l2[t_num].way.insert({t_tag, t_way});
            m_l2[t_num].lrurec[t_loc] = addr;
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = M;
            
            m_l3_ptr->LoadInv(addr, m_id*2 + l1_id, m_id, false);

            cout << "[L2][LoadInv][Located] " << t_loc << endl;
        }
    }

    else{
        //------------------------------------------------ L2 Cache Hit. ------------------------------------------------
        if(!l1_hit)
            ++m_loadinv_hit;
        //Exist in other L1? Kill it.
        if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] != I){
            m_l1_ptr[!l1_id]->UpdateL1(addr, I);
            m_l2[t_num].shadow[t_tag].mesi[!l1_id] = I;
        }

        if(m_l2[t_num].way[t_tag].mesi != S){
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = M;
        }
        else{
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = M;
            m_l3_ptr->LoadInv(addr, m_id*2 + l1_id, m_id, true);
        }
            
        t_loc = m_l2[t_num].way[t_tag].loc;
        m_lru.updlru(m_l2[t_num].lru, t_loc);

        cout << "[L2][LoadInv][Located] " << t_loc << endl;
    }
        

    cout << "[L2][LoadInv][Size] " << m_l2[t_num].way.size() << endl;
}



bool 
L2Cache::Lookup(uint64_t addr)
{
    unsigned int t_num;
    uint64_t t_tag;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;

    if(m_l2[t_num].way.find(t_tag) == m_l2[t_num].way.end())
        return false;
    else
        return true;
}


CacheMESI 
L2Cache::UpdateL2Shadow(uint64_t addr, uint8_t l1_id, CacheMESI mesi)
{
    unsigned int t_num;
    uint64_t t_tag;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;

    if(mesi == I)
        if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] == I){
            m_l2[t_num].shadow.erase(t_tag);
            return I;
        }
        else{
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = I;
            return m_l2[t_num].shadow[t_tag].mesi[!l1_id];
        }
    else{
        m_l2[t_num].shadow[t_tag].mesi[l1_id] = mesi;
        return m_l2[t_num].shadow[t_tag].mesi[!l1_id];
    }
        
}

void 
L2Cache::UpdateL2(uint64_t addr, uint8_t l1_id, CacheMESI l1_mesi, CacheMESI l2_mesi)
{
    unsigned int t_num;
    uint64_t t_tag;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;
    
    if(l1_mesi == I)
        if(m_l2[t_num].shadow[t_tag].mesi[!l1_id] == I)
            m_l2[t_num].shadow.erase(t_tag);
        else
            m_l2[t_num].shadow[t_tag].mesi[l1_id] = I;
    else
        m_l2[t_num].shadow[t_tag].mesi[l1_id] = l1_mesi;

    if(l2_mesi != I)
        m_l2[t_num].way[t_tag].mesi = l2_mesi;
    else{
        m_l2[t_num].lrurec[m_l2[t_num].way[t_tag].loc] = 0;
        m_l2[t_num].way.erase(t_tag);
    }

}



bool
L2Cache::QueryL1L2(uint64_t addr, CacheMESI mesi)
{
    bool t_modify = false;
    unsigned int t_num;
    uint64_t t_tag;

    t_tag = addr >> 15;
    t_num = (addr & L2_OFFSET) >> 6;
    // QueryL2
    if(m_l2[t_num].way.find(t_tag) != m_l2[t_num].way.end()){
        if(m_l2[t_num].way[t_tag].mesi == M)
            t_modify = true;
        if(mesi != I)
            m_l2[t_num].way[t_tag].mesi = mesi;
        else{
            m_l2[t_num].lrurec[m_l2[t_num].way[t_tag].loc] = 0;
            m_l2[t_num].way.erase(t_tag);
        }
    }

    // QueryL1
    if(m_l2[t_num].shadow.find(t_tag) != m_l2[t_num].shadow.end()){
        //L2 Shadow had record.
        if(mesi == I){
            if(m_l2[t_num].shadow[t_tag].mesi[0] != I)
                if(m_l1_ptr[0]->UpdateL1(addr, mesi) == M)
                    t_modify = true;
            if(m_l2[t_num].shadow[t_tag].mesi[1] != I)
                if(m_l1_ptr[1]->UpdateL1(addr, mesi) == M)
                    t_modify = true;
            m_l2[t_num].shadow.erase(t_tag);
        }
        else{
            if(m_l2[t_num].shadow[t_tag].mesi[0] != I){
                if(m_l1_ptr[0]->UpdateL1(addr, mesi) == M)
                    t_modify = true;
                m_l2[t_num].shadow[t_tag].mesi[0] = mesi;
            }
            if(m_l2[t_num].shadow[t_tag].mesi[1] != I){
                if(m_l1_ptr[1]->UpdateL1(addr, mesi) == M)
                    t_modify = true;
                m_l2[t_num].shadow[t_tag].mesi[1] = mesi;
            }
        }
    }

    if(t_modify)
        return true;
    else
        return false;
}
