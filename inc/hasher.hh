#ifndef HASHER_HH
#define HASHER_HH

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include <iostream>
#include <string>

// USEFUL MACROS
// #define HASHER_DEBUG_PRINT
#ifdef HASHER_DEBUG_PRINT
#define HDP(x) x
#else
#define HDP(x)
#endif

#define ST_SET 2048
#define LOG2_ST_SET 11
#define ST_WAY 8

using namespace std;

// SIGNATURE BLOCK
template <typename T>
class SIGNATURE_TABLE_BLOCK
{
public:
    uint8_t valid_;
    uint64_t tag_;
    uint32_t lru_;
    T *data_;

    SIGNATURE_TABLE_BLOCK()
    {
        valid_ = 0;
        tag_ = 0;
        lru_ = 0;
    };
};

template <typename T>
class SIGNATURE_TABLE
{
public:
    const string name_;
    const uint32_t num_set_,
        num_way_,
        num_line_;
    SIGNATURE_TABLE_BLOCK<T> **block_;

    uint64_t access_, hit_, miss_;

    SIGNATURE_TABLE(string name, uint32_t num_set, uint32_t num_way, uint32_t num_line)
        : name_(name), num_set_(num_set), num_way_(num_way), num_line_(num_line), access_(0), hit_(0), miss_(0)
    {
        block_ = new SIGNATURE_TABLE_BLOCK<T> *[num_set_];
        for (uint32_t i = 0; i < num_set_; i++)
        {
            block_[i] = new SIGNATURE_TABLE_BLOCK<T>[num_way_];
            for (uint32_t j = 0; j < num_way_; j++)
                block_[i][j].lru_ = j;
        }
    };

    ~SIGNATURE_TABLE()
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
};

#endif
