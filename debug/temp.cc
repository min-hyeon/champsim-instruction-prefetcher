#include "circular_buffer.h"
#include "../inc/ooo_cpu.h"

CB::CircularBuffer<uint64_t> *circular_buffer = new CB::CircularBuffer<uint64_t>();

void O3_CPU::l1i_prefetcher_cache_operate(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit)
{
    if(cache_hit == 0)
    {
        circular_buffer->enqueue(v_addr);
    }
}