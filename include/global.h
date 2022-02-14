#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

/* ----- Global constant value for SSTable ----- */
const uint64_t MAX_BYTE_SIZE = 1 << 21;
const size_t FILTER_BIT_SIZE = 10 * (1 << 13);
const size_t FILTER_BYTE_SIZE = 10 * (1 << 10);

/* ----- key-value pair of KVStore ----- */
typedef std::pair<uint64_t, std::string> value_type;

/* ----- Indicates a key interval ----- */
typedef std::pair<uint64_t, uint64_t> scope_type;

/* ----- <time_stamp, min_key>, easier way to store a level ----- */
typedef std::pair<uint64_t, uint64_t> key_type;

/* ----- Calculate file size after written to SSTable ----- */
uint64_t cal_size(uint64_t count, uint64_t length);

/* ----- Cross-platform implementation of itoa() ----- */
std::string my_itoa(uint64_t tmp);

/* ----- Check whether a key value is in a certain range ----- */
bool in_scope(std::pair<uint64_t, uint64_t> scope, uint64_t key);

/* ----- Check if a file name ends with .sst ----- */
bool sst_suffix(const char* filePath);


/**
 * Node struct of SkipList & MergeBuffer.
 * put it in global.h to implement a faster constructor of SSTable.
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

/**
 * Store min_key & time_stamp, rewrite operator< to implement a heap.
 * Only used in function merge_table.
 */
struct MergeInfo {
    uint64_t min_key;
    uint64_t time_stamp;
    uint64_t index = 0;
    uint64_t table_index;
    std::ifstream *file_stream;

    MergeInfo(uint64_t mk, uint64_t ts, uint64_t i, uint64_t ti, std::ifstream *fs):
        min_key(mk), time_stamp(ts), index(i), table_index(ti), file_stream(fs) {}

    friend bool operator<(MergeInfo a, MergeInfo b);
};
