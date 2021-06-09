#include <fstream>
#include <cstring>
#include <queue>
#include <iostream>
#include "SSTable.h"
#include "MurmurHash3.h"
#include "utils.h"

uint64_t SSTable::table_id = 0;

SSTable::Header::Header(): time_stamp(0), kv_count(0), min_key(0), max_key(0) {}

SSTable::Header::Header(uint64_t ts, uint64_t kc, uint64_t min, uint64_t max):
    time_stamp(ts), kv_count(kc), min_key(min), max_key(max) {}

SSTable::IndexData::IndexData(): key(0), offset(0) {}

SSTable::IndexData::IndexData(uint64_t k, uint32_t o): key(k), offset(o) {}

void SSTable::bitset_to_bytes(char *buf) {
    memset(buf, 0, FILTER_BYTE_SIZE);
    for (size_t index = 0; index < FILTER_BIT_SIZE; ++index) {
        buf[index >> 3] |= (bloom_filter[index] << (index & 7));
    }
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

    uint64_t kc = data->size();
    uint64_t min = data->begin()->first;
    uint64_t max = (data->end() - 1)->first;
    table_header = Header(ts, kc, min, max);

    file_path = dir + "/" + my_itoa(SSTable::table_id++) + ".sst";

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

        cur_data++;
    }
    string_length = offset;

    // write front header to file
    std::ofstream ssTable_in_file(file_path, std::ios_base::trunc | std::ios_base::binary);
    write_header(ssTable_in_file);

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

    // Generate the remaining data members at the same time
    data_index = new IndexData[kv_count + 1];
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

    file_path = dir + "/" + my_itoa(SSTable::table_id++) + ".sst";

    // write front header to file
    std::ofstream ssTable_in_file(file_path, std::ios_base::trunc | std::ios_base::binary);
    write_header(ssTable_in_file);

    // write string data to file
    cur_node = data_head->next;
    while (cur_node) {
        auto value_size = (long long)(cur_node->value.size());
        auto value_str = cur_node->value.c_str();
        if (value_size == 2) printf("1: %s to %s\n", value_str, file_path.c_str());
        ssTable_in_file.write(value_str, value_size);
        cur_node = cur_node->next;
    }

    ssTable_in_file.close();
}

SSTable::SSTable(const std::string &_file_path) {
    file_path = _file_path;
    std::ifstream cur_SSTable(file_path, std::ios_base::in | std::ios_base::binary);

    cur_SSTable.read((char*)(&table_header), sizeof(Header));
    uint64_t KV_COUNT = table_header.kv_count;

    char *buf = new char[FILTER_BYTE_SIZE];
    cur_SSTable.read(buf, FILTER_BYTE_SIZE);
    bitset_from_bytes(buf);
    delete[] buf;

    data_index = new IndexData[KV_COUNT + 1];
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        cur_SSTable.read((char*)(&(data_index[ind].key)), 8);
        cur_SSTable.read((char*)(&(data_index[ind].offset)), 4);
    }
    
    header_offset = cur_SSTable.tellg();
    cur_SSTable.seekg(0, std::ifstream::end);
    string_length = (size_t)cur_SSTable.tellg() - header_offset;

    cur_SSTable.close();
}

SSTable::~SSTable() {
    delete[] data_index;
}

void SSTable::write_header(std::ofstream &ssTable_in_file) {
    uint64_t KV_COUNT = table_header.kv_count;
    ssTable_in_file.write((char*)(&table_header), sizeof(Header));

    char *bit_seq = new char[FILTER_BYTE_SIZE];
    bitset_to_bytes(bit_seq);
    ssTable_in_file.write(bit_seq, FILTER_BYTE_SIZE);
    delete[] bit_seq;
    
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        ssTable_in_file.write((char*)(&(data_index[ind].key)), 8);
        ssTable_in_file.write((char*)(&(data_index[ind].offset)), 4);

    }
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

scope_type SSTable::get_scope() {
    return std::make_pair(table_header.min_key, table_header.max_key);
}

std::ifstream *SSTable::open_file() {
    auto *target = new std::ifstream(file_path, std::ios_base::in | std::ios_base::binary);
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

std::vector<SSTable*> merge_table(std::vector<SSTable*> &prepared_data, bool is_delete, const std::string &dir) {

    std::priority_queue<MergeInfo> merge_heap;
    std::vector<SSTable*> merged_data;
    MergeBuffer buffer;
    uint64_t max_ts = 0;

    std::ifstream *fs_store[prepared_data.size()];
    
    // initialize min keys
    uint64_t table_index = 0;
    for (auto cur_table_itr : prepared_data) {
        uint64_t mk = cur_table_itr->data_index[0].key;
        uint64_t ts = cur_table_itr->table_header.time_stamp;
        std::ifstream *fs = cur_table_itr->open_file();
        fs_store[table_index] = fs;
        merge_heap.push(MergeInfo(mk, ts, 0, table_index++, fs));

        if (ts > max_ts) max_ts = ts;
    }

    while (!merge_heap.empty()) {
        MergeInfo cur_data = merge_heap.top();
        merge_heap.pop();
        
        uint64_t cur_data_key = cur_data.min_key;
        SSTable *cur_table = prepared_data[cur_data.table_index];
        std::string cur_data_string = cur_table->read_by_index((*cur_data.file_stream), cur_data.index);



        if (cur_data_key != buffer.get_rear()->key || !buffer.get_size()) { 
            // not the same key, just append
            if (!is_delete || cur_data_string != "~DELETED~") {
                // no need / not a delete flag, try to append data
                if (!buffer.push_back(cur_data_key, cur_data_string)) {
                    // space not enough -> save to SSTable (time stamps equal to max_ts)
                    auto *new_table = new SSTable(buffer.get_head(), buffer.get_size(), max_ts, dir);
                    merged_data.push_back(new_table);
                    buffer.clear();
                    // don't forget to push it again
                    buffer.push_back(cur_data_key, cur_data_string);
                }
            } else {
                // delete tag: pop all element with the same key
                while (!merge_heap.empty() && merge_heap.top().min_key == cur_data_key) {
                    MergeInfo deleted_data = merge_heap.top();
                    merge_heap.pop();
                    //set file flag
                    SSTable *cur_tb = prepared_data[deleted_data.table_index];
                    std::string useless_string = cur_tb->read_by_index((*deleted_data.file_stream), deleted_data.index);

                    if (++(deleted_data.index) < cur_table->table_header.kv_count) {
                        // if not the end, set index to next element of current table, and push it back to heap
                        deleted_data.min_key = cur_tb->data_index[deleted_data.index].key;
                        merge_heap.push(deleted_data);
                    }
                }
            }
        }

        // set min key to next element's key, and push it back to heap
        if (++(cur_data.index) < cur_table->table_header.kv_count) {
            // if not the end, set index to next element of current table, and push it back to heap
            cur_data.min_key = cur_table->data_index[cur_data.index].key;
            merge_heap.push(cur_data);
        }
    }

    // push remaining data to SSTable, and write them to Disk when constructing
    if (buffer.get_size() != 0) {
        auto *new_table = new SSTable(buffer.get_head(), buffer.get_size(), max_ts, dir);
        merged_data.push_back(new_table);
    }

    // close all ifstream
    for (auto fs : fs_store) {
        fs->close();
        delete fs;
    }

    // destruct merged tables, delete old SSTables
    for (SSTable *merged_table : prepared_data) {
        merged_table->delete_file();
        delete merged_table;
    }

    return merged_data;
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

std::string SSTable::get_table_path() {
    return file_path;
}
