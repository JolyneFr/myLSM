#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

const uint64_t MAX_BYTE_SIZE = 1 << 21;

typedef std::pair<uint64_t, std::string> value_type;

typedef std::pair<uint64_t, uint64_t> scope_type;

uint64_t cal_size(uint64_t count, uint64_t length);

std::string my_itoa(uint64_t tmp);


/**
 * struct for SkipList
 * put it in global.h to implement a faster constructor of SSTable
 */
struct ListNode {
    uint64_t key = 0;
    std::string value;
    ListNode *prev, *next, *below;

    ListNode(): prev(nullptr), next(nullptr), below(nullptr) {}
    ListNode(const uint64_t &k, std::string v):
        key(k), value(std::move(v)), prev(nullptr), next(nullptr), below(nullptr) {}
    ~ListNode() = default;
    void insertAfterAbove(ListNode *p, ListNode *b);
};

bool isInScope(std::pair<uint64_t, uint64_t> scope, uint64_t key);

struct MergeData {
    uint64_t min_key;
    uint64_t time_stamp;
    uint64_t index = 0;
    uint64_t table_index;
    std::ifstream *file_stream;

    MergeData(uint64_t mk, uint64_t ts, uint64_t i, uint64_t ti,std::ifstream *fs):
        min_key(mk), time_stamp(ts), index(i), table_index(ti), file_stream(fs) {}

    friend bool operator<(MergeData a, MergeData b);
};