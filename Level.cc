#include <queue>
#include "Level.h"
#include "utils.h"

Level::Level(const std::string& dir, size_t l) {
    level_path = dir + "/level-" + my_itoa(l);
}

uint64_t Level::scan_level() {
    std::vector<std::string> dir_list;
    utils::scanDir(level_path, dir_list);
    uint64_t max_ts = 0;

    for (const auto& file_str : dir_list) {
        if (sst_suffix(file_str.c_str())) {
            // if end with .sst, add to level storage
            auto new_ssTable = new SSTable(level_path + "/" + file_str);
            if (new_ssTable->get_time_stamp() > max_ts) {
                // get the max time stamp
                max_ts = new_ssTable->get_time_stamp();
            }
            push_back(new_ssTable);
        }
    }
    return max_ts;
}

Level::~Level() {
    for (auto del_table : level_tables) {
        delete del_table.second;
    }
}

void Level::push_back(SSTable* new_ssTable) {
    key_type table_key = std::make_pair(new_ssTable->get_time_stamp(), new_ssTable->get_scope().first);
    level_tables.insert(std::make_pair(table_key, new_ssTable));
}

size_t Level::get_size() {
    return level_tables.size();
}

std::vector<SSTable*> Level::pop_k(size_t k) {

    std::vector<SSTable*> selected_tables;
    for (size_t ind = 0; ind < k; ++ind) {
        SSTable *popped = level_tables.begin()->second;
        selected_tables.push_back(popped);
        level_tables.erase(level_tables.begin());
    }
    return selected_tables;
}

std::string Level::get(uint64_t key, uint64_t &ret_ts) {
    std::string ret_string;
    size_t find_ind = level_tables.size();
    auto find_itr = level_tables.rbegin();
    // find from tables with bigger time stamp
    while (find_itr != level_tables.rend()) {
        SSTable *cur_tb = find_itr->second;
        if (in_scope(cur_tb->get_scope(), key)) {
            std::string tmp_string = cur_tb->get(key);
            if (!tmp_string.empty()) {
                ret_ts = cur_tb->get_time_stamp();
                return tmp_string; // may be "~DELETED~"
            }
        }
        find_itr++;
    }
    return "";
}

void Level::delete_level() {
    for (auto del_table : level_tables) {
        del_table.second->delete_file();
    }
    utils::rmdir(level_path.c_str());
    level_tables.clear();
}

std::string Level::get_level_path() {
    return level_path;
}

std::map<key_type, SSTable*> *Level::get_level() {
    return &level_tables;
}