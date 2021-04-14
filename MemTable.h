/**
 * @brief Data Structure to implement DRAM storage part of LSM Tree
 *        Implemented by SkipList<K, V>
 *        typeof K = unit64_t; typeof V = std::string
 * @author JolyneFr 519021910390
 */

#pragma once

#include <cstdint>
#include <string>

const uint64_t MAX_BYTE_SIZE = 1 << 21;

class MemTable {

private:

    struct ListNode {
        uint64_t key;
        std::string value;
        ListNode *prev, *next, *below;

        ListNode();
        ListNode(const uint64_t &k, const std::string &v);
        ~ListNode();
        ListNode *insertAfterAbove(ListNode *p, ListNode *b);
    };
    
    uint64_t data_count = 0;  // number of datas in memTable
    uint64_t data_total_length = 0; // total length of all strings

    ListNode *head;

    ListNode *find(uint64_t key);
    uint64_t cal_size(uint64_t count, uint64_t length);

public:
    /**
     * Constructor & destructor for MemTable
     */
    MemTable();

    ~MemTable();

    /**
     * Try to read value marked by key
     * @param key target key number
     * @return target value if key exists 
     *         (An empty string indicates not found.)
     */
    std::string get(uint64_t key);

    /**
     * Put key-value pair into memTable.
     * If the data size would exceed the MemTable limit after the operation,
     * then do not execute put operation.
     * @param key    key to be insert
     * @param value  value to be insert
     * @return Is the operation executed.
     */
    bool put(uint64_t key, std::string value);

    /**
     * Remove key-value pair with certain key
     * @param key   target key for remove
     * @return if the key exists
     */
    bool remove(uint64_t key);

    /**
     * Memory size after this MemTable being generated to .sst file
     * @return memory byte size
     */
    uint64_t mem_size();

    /**
     * Clear all datas in this memTable
     */
    void clear();

    /**
     * For Debug: show SkipList structure
     */
    void show();
};