#pragma once

#include <bitset>
#include <vector>
#include <string>
#include <cstdint>
#include "utils.h"

const size_t FLITER_SIZE = 10 * (1 << 13);

class SSTable {
private:

    struct Header {
        uint64_t time_stamp;
        uint64_t kv_count;
        uint64_t min_key, max_key;
        Header();
        Header(uint64_t ts, uint64_t kc, uint64_t min, uint64_t max);
    } table_header;

    std::bitset<FLITER_SIZE> bloom_fliter;
    
    struct IndexData {
        uint64_t key;
        uint32_t offset;
        IndexData();
        IndexData(uint64_t k, uint32_t o);
    } *data_index;

    std::string data_string;

public:

    SSTable(std::vector<std::pair<uint64_t, std::string>>, uint64_t);
    SSTable(const std::string &file_path);
    ~SSTable();

    /**
     * save SSTable in defined structure
     * @param dir dictonary to stored SSTable
     */
    void writeToFile(const std::string &dir);
};
