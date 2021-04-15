#include <fstream>
#include "SSTable.h"
#include "MurmurHash3.h"

SSTable::Header::Header(): time_stamp(0), kv_count(0), min_key(0), max_key(0) {}

SSTable::Header::Header(uint64_t ts, uint64_t kc, uint64_t min, uint64_t max):
    time_stamp(ts), kv_count(kc), min_key(min), max_key(max) {}

SSTable::IndexData::IndexData(): key(0), offset(0) {}

SSTable::IndexData::IndexData(uint64_t k, uint32_t o): key(k), offset(o) {}

SSTable::SSTable(std::vector<std::pair<uint64_t, std::string>> data, uint64_t ts) {
    uint64_t kc = data.size();
    uint64_t min = data.begin()->first;
    uint64_t max = (data.end() - 1)->first;
    table_header = Header(ts, kc, min, max);

    // Generate the remaining data members at the same time
    data_string = "";
    data_index = new IndexData[kc];
    auto cur_data = data.begin();
    size_t index = 0;
    uint32_t offset = 0;
    while (cur_data != data.end()) {
        uint64_t cur_key = cur_data->first;
        std::string cur_string = cur_data->second;

        // Generate data index
        data_index[index++] = IndexData(cur_key, offset);
        offset += cur_string.size();

        // Configure bloom fliter
        uint32_t hash[4] = {0};
        MurmurHash3_x64_128(&cur_key, sizeof(cur_key), 1, hash);
        bloom_fliter.set(hash[0] % FLITER_SIZE);
        bloom_fliter.set(hash[1] % FLITER_SIZE);
        bloom_fliter.set(hash[2] % FLITER_SIZE);
        bloom_fliter.set(hash[3] % FLITER_SIZE); // Expanding the loop to improve efficiency

        // Append string at the end
        data_string.append(cur_string);

        cur_data++;
    }
}

SSTable::SSTable(const std::string &file_path) {
    
}

SSTable::~SSTable() {
    bloom_fliter.~bitset();
    delete[] data_index;
    data_string.~basic_string();
}

void SSTable::writeToFile(const std::string &dir) {

}