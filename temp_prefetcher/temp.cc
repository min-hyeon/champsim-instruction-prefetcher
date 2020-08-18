#include "ooo_cpu.h"
#include "circular_buffer.h"

#include <list>
#include <memory>

// #define HASHER_DEBUG_PRINT
#ifdef HASHER_DEBUG_PRINT
#define HDP(x) x
#else
#define HDP(x)
#endif

#define ST_SET 2048
#define LOG2_ST_SET 11
#define ST_WAY 8

#define RAS_ENTRY 20
#define RAS_TOP_N_ENTRY 4
#define BHT_ENTRY 50

using namespace std;

class SignatureTableBlock
{
public:
    uint8_t valid_;
    uint64_t tag_;
    uint32_t lru_;
    shared_ptr<uint64_t> data_;

    SignatureTableBlock()
    {
        valid_ = 0;
        tag_ = 0;
        lru_ = 0;
    };
};

class SignatureTable
{
public:
    const string name_;
    const uint32_t num_set_,
        num_way_,
        num_line_;
    SignatureTableBlock **block_;

    uint64_t access_, hit_, miss_;

    SignatureTable(string name, uint32_t num_set, uint32_t num_way, uint32_t num_line)
        : name_(name), num_set_(num_set), num_way_(num_way), num_line_(num_line), access_(0), hit_(0), miss_(0)
    {
        block_ = new SignatureTableBlock *[num_set_];
        for (uint32_t i = 0; i < num_set_; i++)
        {
            block_[i] = new SignatureTableBlock[num_way_];
            for (uint32_t j = 0; j < num_way_; j++)
                block_[i][j].lru_ = j;
        }
    };

    ~SignatureTable()
    {
        for (uint32_t i = 0; i < num_set_; i++)
            delete[] block_[i];
        delete[] block_;
    };

    uint32_t get_set(uint64_t signature),
        get_way(uint64_t signature, uint32_t set),
        lru_victim(uint64_t signature, uint32_t set);

    void lru_update(uint32_t set, uint32_t way);

    uint32_t check_hit(uint64_t signature);

    void handle_read(),
        handle_fill(uint64_t signature, shared_ptr<uint64_t> data),
        handle_prefetch();
};

uint32_t SignatureTable::get_set(uint64_t signature)
{
    return (uint32_t)(signature & ((1 << LOG2_ST_SET) - 1));
}

uint32_t SignatureTable::get_way(uint64_t signature, uint32_t set)
{
    for (uint32_t way = 0; way < ST_WAY; way++)
        if (block_[set][way].valid_ && (block_[set][way].tag_ == (signature >> LOG2_ST_SET) << LOG2_ST_SET))
            return way;
    return ST_WAY;
}

uint32_t SignatureTable::lru_victim(uint64_t signature, uint32_t set)
{
    for (uint32_t way = 0; way < ST_WAY; way++)
        if (block_[set][way].valid_ == false)
        {
            HDP(cout << "[" << name_ << "] " << __func__ << " invalid set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;
                cout << hex << " signature: 0x" << signature << " victim tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;);
            return way;
        }

    for (uint32_t way = 0; way < ST_WAY; way++)
        if (block_[set][way].lru_ == ST_WAY - 1)
        {
            HDP(cout << "[" << name_ << "] " << __func__ << " replace set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;
                cout << hex << " signature: 0x" << signature << " victim tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;);
            return way;
        }

    cerr << "[" << name_ << "] " << __func__ << " no victim! set: " << set << endl;
    assert(0);
}

void SignatureTable::lru_update(uint32_t set, uint32_t way)
{
    for (uint32_t i = 0; i < ST_WAY; i++)
        if (block_[set][i].lru_ < block_[set][way].lru_)
            block_[set][i].lru_++;
    block_[set][way].lru_ = 0;
}

uint32_t SignatureTable::check_hit(uint64_t signature)
{
    uint32_t set = get_set(signature);

    if (set > ST_SET)
    {
        cerr << "[" << name_ << "] " << __func__ << " invalid set index: " << set << " NUM_SET: " << ST_SET;
        assert(0);
    }

    uint32_t way = get_way(signature, set);

    if (way < ST_WAY)
    {
        HDP(cout << "[" << name_ << "] " << __func__ << hex << " signature: 0x" << signature << " tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;
            cout << dec << " set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;);
    }
    else
        HDP(cout << "[" << name_ << "] " << __func__ << hex << " signature: 0x" << signature << dec << " no match! set: " << set << endl;);

    return way;
}

void SignatureTable::handle_read()
{
}

void SignatureTable::handle_fill(uint64_t signature, shared_ptr<uint64_t> data)
{
    uint32_t set = get_set(signature),
             way = get_way(signature, set);

    if (way < ST_WAY)
    {
        block_[set][way].data_ = data;

        hit_++;

        HDP(cout << "[" << name_ << "] " << __func__ << hex << " signature: 0x" << signature << " tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;
            cout << dec << " hit! set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;);
    }
    else
    {
        way = lru_victim(signature, set);

        block_[set][way].valid_ = true;
        block_[set][way].tag_ = (signature >> LOG2_ST_SET) << LOG2_ST_SET;
        lru_update(set, way);

        miss_++;

        HDP(cout << "[" << name_ << "] " << __func__ << hex << " signature: 0x" << signature << dec << " miss! set: " << set << endl;);
    }
    access_++;
}

void SignatureTable::handle_prefetch()
{
}

SignatureTable signature_table("SIGNATURE_TABLE", ST_SET, ST_WAY, ST_SET *ST_WAY);
list<uint64_t> return_address_stack;
list<uint64_t> branch_history_table;
uint64_t prev_signature = 0;
CB::CircularBuffer<uint64_t> *circular_buffer = new CB::CircularBuffer<uint64_t>();

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
        {
            return_address_stack.push_back(ip);
            if (return_address_stack.size() > RAS_ENTRY)
                return_address_stack.pop_front();
        }

        uint64_t signature_set = 0;
        list<uint64_t>::reverse_iterator iter = return_address_stack.rbegin();
        for (uint32_t i = 0; i < RAS_TOP_N_ENTRY; i++)
            signature_set ^= *iter++;

        uint64_t signature_tag = 0;
        for (list<uint64_t>::iterator iter = branch_history_table.begin(); iter != branch_history_table.end(); iter++)
            signature_tag ^= *iter;

        uint64_t signature = (signature_set >> (64 - LOG2_ST_SET)) | (signature_tag << LOG2_ST_SET);
        if(prev_signature != 0)
        {
            signature_table.handle_fill(prev_signature, circular_buffer->dequeue_all().first, circular_buffer->dequeue_all().second);
        }
        prev_signature = signature;

        if (branch_type == BRANCH_RETURN)
            return_address_stack.pop_back();

        branch_history_table.clear();
    }
    else
    {
        branch_history_table.push_back(ip);
        if (branch_history_table.size() > BHT_ENTRY)
            branch_history_table.pop_front();
    }
}

void O3_CPU::l1i_prefetcher_cache_operate(uint64_t v_addr, uint8_t cache_hit, uint8_t prefetch_hit)
{
    if(cache_hit == 0)
    {
        circular_buffer->enqueue(v_addr);
    }
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
