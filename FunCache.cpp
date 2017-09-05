#include "FunCache.h"

FunCache::FunCache(unsigned int core_num)
: core_num(core_num)
{
	m_l3_ptr = new L3Cache(0);

	for(unsigned int i = 0; i < core_num; i++)
	{
		m_l1i_ptr[i] = new L1Cache(2*i);
		m_l1d_ptr[i] = new L1Cache(2*i+1);
		m_l2_ptr[i] = new L2Cache(i);

		m_l1i_ptr[i]->SetL2Ptr(m_l2_ptr[i]);
		m_l1d_ptr[i]->SetL2Ptr(m_l2_ptr[i]);
		m_l1i_ptr[i]->SetL3Ptr(m_l3_ptr);
		m_l1d_ptr[i]->SetL3Ptr(m_l3_ptr);
		m_l2_ptr[i]->SetL3Ptr(m_l3_ptr);

		m_l2_ptr[i]->SetL1Ptr(m_l1i_ptr[i], m_l1d_ptr[i]);
		m_l3_ptr->SetL1Ptr(m_l1i_ptr[i], 2*i);
		m_l3_ptr->SetL1Ptr(m_l1d_ptr[i], 2*i+1);
		m_l3_ptr->SetL2Ptr(m_l2_ptr[i], i);
	}

}

FunCache::~FunCache()
{
	for(unsigned int i = 0; i < core_num; i++)
	{
		delete m_l1i_ptr[i];
		delete m_l1d_ptr[i];
		delete m_l2_ptr[i];
	}
}

void
FunCache::Load(unsigned int l1_id,uint64_t addr)
{
	addr = addr >> 6 << 6;

	if(l1_id%2 == 0)
	{
		m_l1i_ptr[l1_id/2]->Load(addr);
	}
	else
	{
		m_l1d_ptr[l1_id/2]->Load(addr);
	}
}

void
FunCache::Store(unsigned int l1_id,uint64_t addr)
{
	addr = addr >> 6 << 6;

	m_l1d_ptr[l1_id/2]->Store(addr);
}


void
FunCache::Statistics()
{
	for(unsigned int i = 0; i < core_num; i++)
	{
		m_l1i_ptr[i]->Statistics();
		m_l1d_ptr[i]->Statistics();
		m_l2_ptr[i]->Statistics();
	}
	m_l3_ptr->Statistics();
}
