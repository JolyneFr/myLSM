#pragma once

#include <bitset>
#include "global.h"

const size_t FILTER_BIT_SIZE = 10 * (1 << 13);
const size_t FILTER_BYTE_SIZE = 10 * (1 << 10);

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
    char *bitset_to_bytes();
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

    // never store data string in memory
    std::string *data_string = nullptr;

public:

    /**
     * Constructor for SSTable, writing SSTable to level-0 immediately.
     * @param data value vector for all key-value pairs
     * @param time_stamp current SSTable's time stamp
     * @param dir data dictionary of this LSM tree
     */
    SSTable(std::vector<value_type> *data, uint64_t time_stamp, const std::string &dir);

    /**
     * faster & lower memory cost Constructor for SSTable, the low coupling degree is lost.
     * Also writing SSTable to level-0 immediately.
     * @param data_head head node of the lowest level in SkipList, which contains all key-value pairs
     * @param time_stamp current SSTable's time stamp
     * @param dir data dictionary of this LSM tree
     */
    SSTable(ListNode *data_head, uint64_t kv_count, uint64_t time_stamp, const std::string &dir);

    /**
     * Constructor for SSTable from disk, only used when rebuilding LSM tree from dir
     */
    explicit SSTable(const std::string &file_path);
    ~SSTable();

    /**
     * save SSTable in defined structure
     * @param ssTable_in_file oftream to stored SSTable
     */
    void writeHeaderToFile(std::ofstream &ssTable_in_file);

    /**
     * get data stored in current SSTable
     * @return data stored in std::vector
     */
    std::vector<value_type> getData();

    /**
     * find data with given index, use this function to get single data
     * @param index data index 
     * @return current queried string ("" if index >= size)
     */
    std::string get_by_index(uint64_t index);

    friend std::vector<SSTable*> merge(std::vector<SSTable*> prepared_data, uint64_t &time_stamp, bool is_delete, const std::string &dir);

    scope_type getScope();

    uint64_t get_time_stamp();

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

    friend bool operator<(const SSTable& a, const SSTable& b) {
        return a.table_header.time_stamp > b.table_header.time_stamp;
    }
};
