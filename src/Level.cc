#include <queue>
#include <cstdlib>
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
            size_t last_index = file_str.find_last_of('.');
            uint64_t cur_id = std::stoll(file_str.substr(0, last_index));
            if (cur_id >= SSTable::table_id) SSTable::table_id = cur_id + 1;
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
    auto find_itr = level_tables.rbegin();
    // find from tables with bigger time stamp
    while (find_itr != level_tables.rend()) {
        SSTable *cur_tb = find_itr->second;
        if (in_scope(cur_tb->get_scope(), key)) {
            std::string tmp_string = cur_tb->get(key);
            if (!tmp_string.empty()) {
                ret_ts = cur_tb->get_time_stamp();
                return tmp_string; // may be "~DELETED~"
            } else if (level_tables.size() > 2) {
                // if not in the level-0, data overlap is forbidden
                return "";
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

bool Level::check_overlap() {
    std::map<uint64_t, SSTable*> sort_map;
    for (auto itr : level_tables) {
        sort_map.insert(std::make_pair(itr.second->get_scope().first, itr.second));
    }
    uint64_t last_key = 0;
    const char *last_path = "null";
    for (auto itr :sort_map) {
        scope_type s = itr.second->get_scope();
        if (s.first <= last_key && last_key) {
            printf("overlap scope between %s and %s\n",last_path, itr.second->get_table_path().c_str());
            printf("max key before = %llu, min_key = %llu\n", last_key, s.first);
            return false;
        }
        last_key = s.second;
        last_path = itr.second->get_table_path().c_str();
    }
    return true;
}