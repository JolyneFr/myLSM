/**
 * @brief Data Structure to implement DRAM storage part of LSM Tree
 *        Implemented by SkipList<K, V>
 *        typeof K = unit64_t; typeof V = std::string
 *        Uncommon SkipList: has size limit
 * @author JolyneFr 519021910390
 */

#pragma once

#include "global.h"

class SkipList {

private:
    
    uint64_t data_count = 0;  // number of datas in memTable
    uint64_t data_total_length = 0; // total length of all strings

    ListNode *head;

    ListNode *lowest_head;

    ListNode *find(uint64_t key);

public:
    /**
     * default constructor & destructor for MemTable
     */
    SkipList();
    ~SkipList();

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
     * @param key key to be insert
     * @param value value to be insert
     * @return Is the operation executed.
     */
    bool put(uint64_t key, std::string value);

    /**
     * Remove key-value pair with certain key
     * @param key target key for remove
     * @return if the key exists
     */
    bool remove(uint64_t key);

    /**
     * get datas in SkipList in the order of key value undecreased.
     * @return std vector that store all datas.
     */
    std::vector<value_type> *exported_data();

    /**
     * get the lowest head node
     * @return the head node at the bottom
     */
    ListNode *get_bottom_head();

    /**
     * Memory size after this MemTable being generated to .sst file
     * @return memory byte size
     */
    uint64_t mem_size();

    /**
     * get the number of key-value pair in SkipList
     * @return size of SkipList
     */
    uint64_t get_kv_count();

    /**
     * Clear all datas in this memTable
     */
    void clear();

    /**
     * For Debug: show SkipList structure
     */
    void show();
};