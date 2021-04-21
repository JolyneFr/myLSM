#include <fstream>
#include <iostream>
#include <cstring>
#include <queue>
#include "SSTable.h"
#include "MurmurHash3.h"
#include "utils.h"

SSTable::Header::Header(): time_stamp(0), kv_count(0), min_key(0), max_key(0) {}

SSTable::Header::Header(uint64_t ts, uint64_t kc, uint64_t min, uint64_t max):
    time_stamp(ts), kv_count(kc), min_key(min), max_key(max) {}

SSTable::IndexData::IndexData(): key(0), offset(0) {}

SSTable::IndexData::IndexData(uint64_t k, uint32_t o): key(k), offset(o) {}

char *SSTable::bitset_to_bytes() {
    char *buf = new char[FILTER_BYTE_SIZE];
    memset(buf, 0, FILTER_BYTE_SIZE);
    for (size_t index = 0; index < FILTER_BIT_SIZE; ++index) {
        buf[index >> 3] |= (bloom_filter[index] << (index & 7));
    }
    return buf;
}

void SSTable::bitset_from_bytes(const char* buf) {
    for (size_t index = 0; index < FILTER_BIT_SIZE; ++index) {
        bloom_filter[index] = ((buf[index >> 3] >> (index & 7)) & 1);
    }
}

bool SSTable::bloom_test(uint64_t key) {
    uint32_t cur_hash[4] = {0};
    MurmurHash3_x64_128(&key, sizeof(key), 1, cur_hash);

    return (bloom_filter.test(cur_hash[0] % FILTER_BIT_SIZE) &&
            bloom_filter.test(cur_hash[1] % FILTER_BIT_SIZE) &&
            bloom_filter.test(cur_hash[2] % FILTER_BIT_SIZE) &&
            bloom_filter.test(cur_hash[3] % FILTER_BIT_SIZE));
}

size_t SSTable::binary_search(uint64_t key) {
    size_t left = 0, right = table_header.kv_count - 1;
    while (left <= right) {
        size_t mid = (left + right) >> 1;
        uint64_t mid_key = data_index[mid].key;
        if (mid_key == key) return mid;
        if (mid_key > key) right = mid - 1;
        else left = mid + 1;
    }
    return table_header.kv_count;
}

SSTable::SSTable(std::vector<value_type> *data, uint64_t ts, const std::string &dir) {

    file_path = dir + my_itoa(ts) + ".sst";

    uint64_t kc = data->size();
    uint64_t min = data->begin()->first;
    uint64_t max = (data->end() - 1)->first;
    table_header = Header(ts, kc, min, max);

    header_offset = cal_size(kc, 0);

    // Generate the remaining data members at the same time
    data_index = new IndexData[kc];
    auto cur_data = data->begin();
    size_t index = 0;
    uint32_t offset = 0;
    while (cur_data != data->end()) {
        uint64_t cur_key = cur_data->first;

        // Generate data index
        data_index[index] = IndexData(cur_key, offset);
        offset += cur_data->second.size();

        // Configure bloom filter
        uint32_t hash[4] = {0};
        MurmurHash3_x64_128(&cur_key, sizeof(cur_key), 1, hash);
        bloom_filter.set(hash[0] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[1] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[2] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[3] % FILTER_BIT_SIZE); // Expanding the loop to improve efficiency

        // Append string at the end (never used)
        //data_string[index++] = cur_string;

        cur_data++;
    }
    string_length = offset;

    // write front header to file
    std::ofstream ssTable_in_file(file_path, std::ios_base::trunc);
    writeHeaderToFile(ssTable_in_file);

    // write string data to file
    cur_data = data->begin();
    while (cur_data != data->end()) {
        ssTable_in_file.write(cur_data->second.c_str(), cur_data->second.size());
        cur_data++;
    }

    ssTable_in_file.close();
    delete data;
}

SSTable::SSTable(ListNode *data_head, uint64_t kv_count, uint64_t ts, const std::string &dir) {
    // dir includes data and level
    file_path = dir + my_itoa(ts) + ".sst";

    // Generate the remaining data members at the same time
    data_index = new IndexData[kv_count];
    ListNode *cur_node = data_head;
    size_t index = 0;
    uint32_t offset = 0;
    while (cur_node->next) {
        cur_node = cur_node->next;
        uint64_t cur_key = cur_node->key;

        // Generate data index
        data_index[index++] = IndexData(cur_key, offset);
        offset += cur_node->value.size();

        // Configure bloom filter
        uint32_t hash[4] = {0};
        MurmurHash3_x64_128(&cur_key, sizeof(cur_key), 1, hash);
        bloom_filter.set(hash[0] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[1] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[2] % FILTER_BIT_SIZE);
        bloom_filter.set(hash[3] % FILTER_BIT_SIZE); // Expanding the loop to improve efficiency
    }
    string_length = offset;

    uint64_t min = data_head->next->key;
    uint64_t max = cur_node->key;
    table_header = Header(ts, kv_count, min, max);
    header_offset = cal_size(kv_count, 0);

    // write front header to file
    std::ofstream ssTable_in_file(file_path, std::ios_base::trunc);
    writeHeaderToFile(ssTable_in_file);

    // write string data to file
    cur_node = data_head->next;
    while (cur_node) {
        ssTable_in_file.write(cur_node->value.c_str(), cur_node->value.size());
        cur_node = cur_node->next;
    }

    ssTable_in_file.close();
}

SSTable::SSTable(const std::string &file_path) {
    std::ifstream cur_SSTable(file_path);

    cur_SSTable.read((char*)(&table_header), sizeof(Header));
    uint64_t KV_COUNT = table_header.kv_count;

    char *buf = new char[FILTER_BYTE_SIZE];
    cur_SSTable.read(buf, FILTER_BYTE_SIZE);
    bitset_from_bytes(buf);
    delete[] buf;

    data_index = new IndexData[KV_COUNT];
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        cur_SSTable.read((char*)(&(data_index[ind].key)), 8);
        cur_SSTable.read((char*)(&(data_index[ind].offset)), 4);
    }

    
    header_offset = cur_SSTable.tellg();
    cur_SSTable.seekg(0, std::ifstream::end);
    string_length = (size_t)cur_SSTable.tellg() - header_offset;
    /* no need to read all string from file
    cur_SSTable.seekg(front_length);
    data_string = new std::string[KV_COUNT];
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        size_t cur_length = (ind != KV_COUNT - 1) ? 
            data_index[ind + 1].offset - data_index[ind].offset : 
            string_size - data_index[ind].offset;

        char *str_buf = new char[cur_length];
        cur_SSTable.read(str_buf, cur_length);
        data_string[ind] = std::string(str_buf, cur_length);
        delete[] str_buf;
    }
    */

    cur_SSTable.close();
}

SSTable::~SSTable() {
    delete[] data_index;
}

void SSTable::writeHeaderToFile(std::ofstream &ssTable_in_file) {
    uint64_t KV_COUNT = table_header.kv_count;
    ssTable_in_file.write((char*)(&table_header), sizeof(Header));

    char *bit_seq = bitset_to_bytes();
    ssTable_in_file.write(bit_seq, FILTER_BYTE_SIZE);
    delete[] bit_seq;
    
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        ssTable_in_file.write((char*)(&(data_index[ind].key)), 8);
        ssTable_in_file.write((char*)(&(data_index[ind].offset)), 4);
    }

    /* don't write data string here
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        cur_SSTable.write(data_string[ind].c_str(), data_string[ind].size());
    }
    */
}

std::vector<std::pair<uint64_t, std::string>> SSTable::getData() {
    std::vector<std::pair<uint64_t, std::string>> data_set;
    for (size_t ind = 0; ind < table_header.kv_count; ++ind) {
        data_set.emplace_back(data_index[ind].key, data_string[ind]);
    }
    return data_set;
}

std::string SSTable::get_by_index(uint64_t index) {

    size_t KV_COUNT = table_header.kv_count;

    if (index >= KV_COUNT) 
        return "";

    size_t cur_offset = data_index[index].offset;
    std::ifstream ssTable_in_file(file_path);
    ssTable_in_file.seekg(header_offset + cur_offset);
    size_t cur_length = (index != KV_COUNT - 1) ? 
            data_index[index + 1].offset - cur_offset : 
            string_length - cur_offset;

    char *str_buf = new char[cur_length];
    ssTable_in_file.read(str_buf, cur_length);
    std::string cur_data(str_buf, cur_length);
    delete[] str_buf;

    return cur_data;
}

scope_type SSTable::getScope() {
    return std::make_pair(table_header.min_key, table_header.max_key);
}

std::ifstream *SSTable::open_file() {
    auto *target = new std::ifstream(file_path);
    target->seekg(header_offset);
    return target;
}

std::string SSTable::read_by_index(std::ifstream &fs, uint64_t index) {
    size_t cur_offset = data_index[index].offset;
    size_t cur_length = (index != table_header.kv_count - 1) ? 
            data_index[index + 1].offset - cur_offset : 
            string_length - cur_offset;

    char *str_buf = new char[cur_length];
    fs.read(str_buf, cur_length);
    std::string cur_data(str_buf, cur_length);
    delete[] str_buf;

    return cur_data;
}

uint64_t SSTable::get_time_stamp() const {
    return table_header.time_stamp;
}

std::vector<SSTable*> merge_table(std::vector<SSTable*> prepared_data, uint64_t &time_stamp, bool is_delete, const std::string &dir) {

    std::priority_queue<MergeData> merge_heap;
    std::vector<SSTable*> merged_data;
    MergeBuffer buffer;
    
    // initialize min keys
    uint64_t table_index = 0;
    auto cur_table_itr = prepared_data.begin();
    while (cur_table_itr != prepared_data.end()) {
        uint64_t mk = (*cur_table_itr)->data_index[0].key;
        uint64_t ts = (*cur_table_itr)->table_header.time_stamp;
        std::ifstream *fs = (*cur_table_itr)->open_file();
        merge_heap.push(MergeData(mk, ts, 0, table_index++,fs));
        cur_table_itr++;
    }

    while (!merge_heap.empty()) {
        MergeData cur_data = merge_heap.top();
        merge_heap.pop();
        
        uint64_t cur_data_key = cur_data.min_key;
        SSTable *cur_table = prepared_data[cur_data.table_index];
        std::string cur_data_string = cur_table->read_by_index((*cur_data.file_stream), cur_data.index);

        if (cur_data_key != buffer.get_rear()->key || !buffer.get_size()) { 
            // not the same key, just append
            if (!is_delete || cur_data_string != "~DELETED~") {
                // no need / not a delete flag, try to append data
                if (!buffer.push_back(cur_data_key, cur_data_string)) {
                    // space not enough -> save to SSTable
                    auto *new_table = new SSTable(buffer.get_head(), buffer.get_size(), time_stamp++, dir);
                    merged_data.push_back(new_table);
                    buffer.clear();
                    // don't forget to push it again
                    buffer.push_back(cur_data_key, cur_data_string);
                }
            } else {
                // delete tag: pop all element with the same key
                while (!merge_heap.empty() && merge_heap.top().min_key == cur_data_key) {
                    MergeData deleted_data = merge_heap.top();
                    merge_heap.pop();
                    //set file flag
                    SSTable *cur_tb = prepared_data[deleted_data.table_index];
                    std::string useless_string = cur_tb->read_by_index((*deleted_data.file_stream), deleted_data.index);

                    if (++(deleted_data.index) != cur_table->table_header.kv_count) {
                        // if not the end, set index to next element of current table, and push it back to heap
                        deleted_data.min_key = cur_tb->data_index[deleted_data.index].key;
                        merge_heap.push(deleted_data);
                    }
                }
            }
        }

        // set min key to next element's key, and push it back to heap
        if (++(cur_data.index) != cur_table->table_header.kv_count) {
            // if not the end, set index to next element of current table, and push it back to heap
            cur_data.min_key = cur_table->data_index[cur_data.index].key;
            merge_heap.push(cur_data);
        }
    }

    // push remaining data to SSTable, and write them to Disk when constructing
    if (buffer.get_size() != 0) {
        auto *new_table = new SSTable(buffer.get_head(), buffer.get_size(), time_stamp++, dir);
        merged_data.push_back(new_table);
    }

    // destruct merged tables, delete old SSTables
    for (SSTable *merged_table : prepared_data) {
        delete merged_table;
    }

    return merged_data;
}

bool operator<(const SSTable& a, const SSTable& b) {
    return a.table_header.time_stamp > b.table_header.time_stamp;
}

std::string SSTable::get(uint64_t key) {
    if (bloom_test(key)) {
        size_t ind = binary_search(key);
        if (ind != table_header.kv_count) {
            return get_by_index(ind); // may be "~DELETED~"
        }
    }
    return "";
}

void SSTable::delete_file() {
    utils::rmfile(file_path.c_str());
}