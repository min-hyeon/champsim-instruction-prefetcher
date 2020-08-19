#ifndef SIGNATURE_TABLE_HH
#define SIGNATURE_TABLE_HH

#include <stdint.h>

#include <iostream>
#include <list>

#define HASHER_DEBUG_PRINT
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

    list<uint64_t> *data_;

    SignatureTableBlock() : valid_(0), tag_(0), lru_(0), data_(NULL){};
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

    list<uint64_t> *get_data(uint64_t signature);

    void lru_update(uint32_t set, uint32_t way);

    uint32_t check_hit(uint64_t signature);

    void handle_fill(uint64_t signature, list<uint64_t> *data);
};

uint32_t SignatureTable::get_set(uint64_t signature)
{
    return (uint32_t)(signature & ((1 << LOG2_ST_SET) - 1));
}

uint32_t SignatureTable::get_way(uint64_t signature, uint32_t set)
{
    for (uint32_t way = 0; way < ST_WAY; way++)
        if (block_[set][way].valid_ && block_[set][way].tag_ == (signature >> LOG2_ST_SET))
            return way;

    return ST_WAY;
}

uint32_t SignatureTable::lru_victim(uint64_t signature, uint32_t set)
{
    uint32_t way;

    for (way = 0; way < ST_WAY; way++)
        if (block_[set][way].valid_ == false)
        {
            HDP(cout << "[" << name_ << "] " << __func__ << " invalid set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;
                cout << hex << " signature: 0x" << signature << " victim tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_ << endl;);

            break;
        }

    if (way == ST_WAY)
        for (way = 0; way < ST_WAY; way++)
            if (block_[set][way].lru_ == ST_WAY - 1)
            {
                HDP(cout << "[" << name_ << "] " << __func__ << " replace set: " << set << " way: " << way << " lru: " << block_[set][way].lru_;
                    cout << hex << " signature: 0x" << signature << " victim tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_ << endl;);

                break;
            }

    return way;
}

void SignatureTable::lru_update(uint32_t set, uint32_t way)
{
    for (uint32_t i = 0; i < ST_WAY; i++)
        if (block_[set][i].lru_ < block_[set][way].lru_)
            block_[set][i].lru_++;
    block_[set][way].lru_ = 0;
}

list<uint64_t> *SignatureTable::get_data(uint64_t signature)
{
    uint32_t set = get_set(signature),
             way = get_way(signature, set);

    return block_[set][way].data_;
}

uint32_t SignatureTable::check_hit(uint64_t signature)
{
    uint32_t set = get_set(signature),
             way = get_way(signature, set);

    if (way < ST_WAY)
    {
        HDP(cout << "[" << name_ << "] " << __func__ << hex << " hit! signature: 0x" << signature << " tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;
            cout << dec << " set: " << set << " way: " << way << " lru: " << block_[set][way].lru_ << endl;);
    }
    else
        HDP(cout << "[" << name_ << "] " << __func__ << hex << " miss! signature: 0x" << signature << dec << " set: " << set << endl;);

    return way;
}

void SignatureTable::handle_fill(uint64_t signature, list<uint64_t> *data)
{
    uint32_t set = get_set(signature),
             way = get_way(signature, set);

    if (way < ST_WAY)
    {
        block_[set][way].data_ = data;

        hit_++;

        HDP(cout << "[" << name_ << "] " << __func__ << hex << " hit! signature: 0x" << signature << " tag: 0x" << block_[set][way].tag_ << " data: 0x" << block_[set][way].data_;
            cout << dec << " hit! set: " << set << " way: " << way << " lru: " << block_[set][way].lru_ << endl;);
    }
    else
    {
        way = lru_victim(signature, set);

        block_[set][way].valid_ = true;
        block_[set][way].tag_ = (signature >> LOG2_ST_SET);
        lru_update(set, way);
        block_[set][way].data_ = data;

        miss_++;

        HDP(cout << "[" << name_ << "] " << __func__ << hex << " miss! signature: 0x" << signature << dec << " set: " << set << endl;);
    }
    access_++;
}

#endif
