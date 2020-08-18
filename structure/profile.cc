
#include "graph/graph.hh"

#include <tuple>

typedef void (*p_l1i_prefetcher_initialize)();
typedef void (*p_l1i_prefetcher_branch_operate)(uint64_t ip, uint8_t branch_type, uint64_t branch_target);
typedef void (*p_l1i_prefetcher_cache_operate)(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit);
typedef void (*p_l1i_prefetcher_cycle_operate)();
typedef void (*p_l1i_prefetcher_cache_fill)(uint64_t v_addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_v_addr);
typedef void (*p_l1i_prefetcher_final_stats)();

using ip_t                   = uint64_t;
using branch_type_t          = uint8_t;
using branch_target_t        = uint64_t;
using cache_hit_t            = uint8_t;
using prefetch_hit_t         = uint8_t;
using cache_v_addr_t         = uint64_t;
using cahce_set_t            = uint32_t;
using cahce_way_t            = uint32_t;
using cache_prefetch_t       = uint8_t;
using cache_evicted_v_addr_t = uint64_t;
