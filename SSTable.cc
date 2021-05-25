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
    }
    string_length = offset;

    uint64_t min = data_head->next->key;
    uint64_t max = cur_node->key;
    table_header = Header(ts, kv_count, min, max);
    header_offset = cal_header_offset(kv_count);

    file_path = dir + "/" + my_itoa(ts) + "-" + my_itoa(min) + ".sst";

    // write front header to file
    std::ofstream ssTable_in_file(file_path, std::ios_base::trunc | std::ios_base::binary);
    write_header(ssTable_in_file);

    // write string data to file
    cur_node = data_head->next;
    while (cur_node) {
        ssTable_in_file.write(cur_node->value.c_str(), (long long)(cur_node->value.size()));
        cur_node = cur_node->next;
    }
    delete[] data_index;
    data_index = nullptr;

    ssTable_in_file.close();
}

SSTable::SSTable(const std::string &_file_path) {
    file_path = _file_path;
    std::ifstream cur_SSTable(file_path, std::ios_base::in | std::ios_base::binary);

    cur_SSTable.read((char*)(&table_header), sizeof(Header));
    uint64_t KV_COUNT = table_header.kv_count;

    data_index = nullptr;
    
    header_offset = cal_header_offset(KV_COUNT);
    cur_SSTable.seekg(0, std::ifstream::end);
    string_length = (size_t)cur_SSTable.tellg() - header_offset;

    cur_SSTable.close();
}

SSTable::~SSTable() {
    delete[] data_index;
    data_index = nullptr;
}

void SSTable::write_header(std::ofstream &ssTable_in_file) {
    uint64_t KV_COUNT = table_header.kv_count;
    ssTable_in_file.write((char*)(&table_header), sizeof(Header));
    
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        ssTable_in_file.write((char*)(&(data_index[ind].key)), 8);
        ssTable_in_file.write((char*)(&(data_index[ind].offset)), 4);
    }
}

void SSTable::read_index(std::ifstream *fs) {
    fs->seekg(32, std::ios_base::beg);
    uint64_t KV_COUNT = table_header.kv_count;

    data_index = new IndexData[KV_COUNT + 1];
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        fs->read((char*)(&(data_index[ind].key)), 8);
        fs->read((char*)(&(data_index[ind].offset)), 4);
    }
}

void SSTable::delete_index() {
    delete[] data_index;
    data_index = nullptr;
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
        uint64_t mk = cur_table_itr->table_header.min_key;
        uint64_t ts = cur_table_itr->table_header.time_stamp;
        std::ifstream *fs = cur_table_itr->open_file();
        cur_table_itr->read_index(fs);
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
    std::ifstream cur_SSTable(file_path, std::ios_base::in | std::ios_base::binary);
    uint64_t KV_COUNT = table_header.kv_count, cur_key;
    uint32_t cur_offset;
    cur_SSTable.seekg(sizeof(Header));
    for (size_t ind = 0; ind < KV_COUNT; ++ind) {
        cur_SSTable.read((char*)(&(cur_key)), 8);
        cur_SSTable.read((char*)(&(cur_offset)), 4);
        if (cur_key == key) {
            size_t cur_length;
            if (ind != KV_COUNT - 1) {
                cur_SSTable.seekg(8, std::ios_base::cur);
                uint32_t next_offset;
                cur_SSTable.read((char*)(&(next_offset)), 4);
                cur_length = next_offset - cur_offset;
            } else {
                cur_length = string_length - cur_offset;
            }
            cur_SSTable.seekg((long long)header_offset + cur_offset);
            char *buf = new char[cur_length];
            cur_SSTable.read(buf, (long long)cur_length);
            std::string str(buf, cur_length);
            cur_SSTable.close();
            delete[] buf;
            return str;
        }
    }
    cur_SSTable.close();
    return "";
}

void SSTable::delete_file() {
    utils::rmfile(file_path.c_str());
}

uint64_t SSTable::cal_header_offset(uint64_t kv) {
    return 32 + 12 * kv;
}
