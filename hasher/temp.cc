#include "ooo_cpu.h"
#include "hasher.cc"

#include <list>

SIGNATURE_TABLE<uint64_t *> ST("SIGNATURE_TABLE", ST_SET, ST_WAY, ST_SET *ST_WAY);
list<uint64_t> RAS;
list<uint64_t> BHT;
uint64_t *MISS_HISTORY;

void O3_CPU::l1i_prefetcher_initialize()
{
}

void O3_CPU::l1i_prefetcher_branch_operate(uint64_t ip, uint8_t branch_type, uint64_t branch_target)
{
    if (branch_type == NOT_BRANCH)
        return;

    if (branch_type == BRANCH_DIRECT_CALL || branch_type == BRANCH_INDIRECT_CALL || branch_type == BRANCH_RETURN)
    {
        if (branch_type == BRANCH_DIRECT_CALL || branch_type == BRANCH_INDIRECT_CALL)
            RAS.push_back(ip);

        uint64_t signature_set = 0;
        list<uint64_t>::reverse_iterator iter = RAS.rbegin();
        for (uint32_t i = 0; i < RAS_ENTRY; i++)
            signature_set ^= *iter++;

        uint64_t signature_tag = 0;
        for (list<uint64_t>::iterator iter = BHT.begin(); iter != BHT.end(); iter++)
            signature_tag ^= *iter;

        uint64_t signature = (signature_set >> (64 - LOG2_ST_SET)) | (signature_tag << LOG2_ST_SET);
        ST.handle_fill(signature, MISS_HISTORY);

        if (branch_type == BRANCH_RETURN)
            RAS.pop_back();

        BHT.clear();
    }
    else
    {
        BHT.push_back(ip);
        if (BHT.size() > BHT_ENTRY)
            BHT.pop_front();
    }
}

void O3_CPU::l1i_prefetcher_cache_operate(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit)
{
}

void O3_CPU::l1i_prefetcher_cycle_operate()
{
}

void O3_CPU::l1i_prefetcher_cache_fill(uint64_t v_addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_v_addr)
{
}

void O3_CPU::l1i_prefetcher_final_stats()
{
}
