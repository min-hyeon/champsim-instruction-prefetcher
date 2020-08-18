#include "hasher.hh"

template <typename T>
uint32_t SIGNATURE_TABLE<T>::get_set(uint64_t signature)
{
    return (uint32_t)(signature & ((1 << LOG2_ST_SET) - 1));
}

template <typename T>
uint32_t SIGNATURE_TABLE<T>::get_way(uint64_t signature, uint32_t set)
{
    for (uint32_t way = 0; way < ST_WAY; way++)
        if (block_[set][way].valid_ && (block_[set][way].tag_ == signature))
            return way;
    return ST_WAY;
}

template <typename T>
uint32_t SIGNATURE_TABLE<T>::lru_victim(uint64_t signature, uint32_t set)
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

template <typename T>
void SIGNATURE_TABLE<T>::lru_update(uint32_t set, uint32_t way)
{
    for (uint32_t i = 0; i < NUM_WAY; i++)
        if (block_[set][i].lru_ < block_[set][way].lru_)
            block_[set][i].lru_++;
    block_[set][way].lru_ = 0;
}

template <typename T>
uint32_t SIGNATURE_TABLE<T>::check_hit(uint64_t signature)
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

template <typename T>
void SIGNATURE_TABLE<T>::handle_read()
{
}

template <typename T>
void SIGNATURE_TABLE<T>::handle_fill(uint64_t signature, T data)
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
        block_[set][way].tag_ = (signature << LOG2_ST_SET) >> LOG2_ST_SET;
        lru_update(set, way);

        miss_++;

        HDP(cout << "[" << name_ << "] " << __func__ << hex << " signature: 0x" << signature << dec << " miss! set: " << set << endl;);
    }
    access_++;
}

template <typename T>
void SIGNATURE_TABLE<T>::handle_prefetch()
{
}
