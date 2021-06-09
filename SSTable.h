#pragma once

#include <bitset>
#include "MergeBuffer.h"

class SSTable {
private:

    std::string file_path;

    uint64_t header_offset;
    uint64_t string_length;

    struct Header {
        uint64_t time_stamp;
        uint64_t kv_count;
        uint64_t min_key, max_key;
        Header();
        Header(uint64_t ts, uint64_t kc, uint64_t min, uint64_t max);
    } table_header;

    std::bitset<FILTER_BIT_SIZE> bloom_filter;
    void bitset_to_bytes(char*);
    void bitset_from_bytes(const char*);
    bool bloom_test(uint64_t);
    
    struct IndexData {
        uint64_t key;
        uint32_t offset;
        IndexData();
        IndexData(uint64_t k, uint32_t o);
    } *data_index;
    /**
     * binary search in data_index for key
     * @param key query key
     * @return The index corresponding to the key (kv_count if not found)
     */
    size_t binary_search(uint64_t);

public:
    /*
     * Unique SSTable ID, start by 0.
     */
    static uint64_t table_id;

    /**
     * Constructor for SSTable, writing SSTable to level-0 immediately.
     * @param data value vector for all key-value pairs
     * @param time_stamp current SSTable's time stamp
     * @param dir data dictionary of this LSM tree
     */
    SSTable(std::vector<value_type> *data, uint64_t time_stamp, const std::string &dir);

    /**
     * Faster & lower memory cost Constructor for SSTable, the low coupling degree is lost.
     * Also writing SSTable to level-0 immediately.
     * @param data_head head node of the lowest level in SkipList, which contains all key-value pairs
     * @param time_stamp current SSTable's time stamp
     * @param dir data dictionary of this LSM tree
     */
    SSTable(ListNode *data_head, uint64_t kv_count, uint64_t time_stamp, const std::string &dir);

    /**
     * Constructor for SSTable from disk, only used when rebuilding LSM tree from dir.
     */
    explicit SSTable(const std::string &file_path);
    /**
     * Destructor, does not delete SSTable file for persistence.
     */
    ~SSTable();

    /**
     * Save SSTable in defined structure.
     * @param ssTable_in_file oftream to stored SSTable
     */
    void write_header(std::ofstream &ssTable_in_file);

    /**
     * Find data with given index, use this function to get single data.
     * @param index data index 
     * @return current queried string ("" if index >= size)
     */
    std::string get_by_index(uint64_t index);

    /**
     * Merge several SSTables and write them to Disk at the same time.
     * @param prepared_data SSTables to be merged
     * @param time_stamp current time stamp to initialise new SSTable
     * @param is_delete if true, delete all data with "~DELETED~" flag
     * @param dir target write dictionary
     * @return merged SSTables
     */
    friend std::vector<SSTable*> merge_table(std::vector<SSTable*> &prepared_data, bool is_delete, const std::string &dir);

    /**
     * @return pair of (min_key, max_keu), which indicates range of data in this SSTable.
     */
    scope_type get_scope();

    /**
     * @return time stamp of current SSTable
     */
    uint64_t get_time_stamp() const;

    /**
     * open the SSTable file to prepare for reading data continuously
     * @return quote to ifstream of SSTable, set flag position to the beginning of data area
     */
    std::ifstream *open_file();

    /**
     * read data with given ifstream and index, use this function to read data continuously
     * @param fs quote to ifstream of SSTable, flag set to the beginning of current data
     * @param index data index
     * @return current queried string ("" if index >= size)
     */
    std::string read_by_index(std::ifstream &fs, uint64_t index);

    /**
     * Get string by key (if any).
     * Bloom test -> binary search -> read from linked file.
     * @param key queried key value
     * @return target string ("" if not exist)
     */
    std::string get(uint64_t key);

    /**
     * Delete file linked with current SSTable.
     */
    void delete_file();

    std::string get_table_path();
};
