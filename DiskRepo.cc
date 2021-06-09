#include "DiskRepo.h"
#include "utils.h"
#include <iostream>

DiskRepo::DiskRepo(const std::string& d): time_stamp(1), dir(d) {
    if (!utils::dirExists(dir)) {
        utils::mkdir(d.c_str());
    } else {
        // read data from existing SSTable
        std::vector<std::string> dir_list;
        utils::scanDir(d, dir_list);
        for (size_t i = 0; ; ++i) {
            std::string cur_match_name = "level-" + my_itoa(i);
            auto dir_str = dir_list.begin();
            while (dir_str != dir_list.end()) {
                if (cur_match_name == (*dir_str)) { // hit!
                    // scan all files in current level
                    auto new_level = new Level(dir, i);
                    uint64_t max_ts = new_level->scan_level();
                    if (time_stamp <= max_ts) time_stamp = max_ts;
                    disk_levels.push_back(new_level);
                    break;
                }
                dir_str++;
            }
            if (dir_str == dir_list.end()) break;
        }
        time_stamp++;
    }
}

DiskRepo::~DiskRepo() {
    for (auto level : disk_levels) {
        delete level;
    }
}

void DiskRepo::create_level(uint64_t ls) {
    std::string level_str = dir + "/level-" + my_itoa(ls);
    utils::mkdir(level_str.c_str());
    disk_levels.push_back(new Level(dir, ls));
}

void DiskRepo::handle_overflow(size_t overflowed_index) {
    Level *upper_level = disk_levels[overflowed_index];
    std::vector<SSTable*> overflowed_tables;

    if (overflowed_index == 0) { // pop SSTable out to prepare for merging
        // level-0: compact all
        overflowed_tables = upper_level->pop_k(upper_level->get_size());
    } else {
        // not level-0
        size_t merge_num = upper_level->get_size() - (1 << (overflowed_index + 1));
        overflowed_tables = upper_level->pop_k(merge_num);
    }

    if (overflowed_index == disk_levels.size() - 1) { // next dir doesn't exists
        create_level(overflowed_index + 1);
    }

    Level *next_level = disk_levels[overflowed_index + 1];
    // merge a table from upper level at once => maybe faster

    scope_type overflow_scope = std::make_pair(UINT64_MAX, 0);

    for (auto cur_table : overflowed_tables) {
        auto cur_scope = cur_table->get_scope();
        if (cur_scope.first < overflow_scope.first)
            overflow_scope.first = cur_scope.first;
        if (cur_scope.second > overflow_scope.second)
            overflow_scope.second = cur_scope.second;
    }

    auto level_path = next_level->get_level_path();
    auto next_lv_map = next_level->get_level();

    auto find_itr = next_lv_map->begin();
    std::vector<std::map<key_type, SSTable*>::iterator> del_record;
    while (find_itr != next_lv_map->end()) {
        auto cur_table = find_itr->second;
        if (overflow_scope.second >= cur_table->get_scope().first &&
            overflow_scope.first <= cur_table->get_scope().second) {
            del_record.push_back(find_itr);
            overflowed_tables.push_back(cur_table);
        }
        find_itr++;
    }

    for (auto del_itr : del_record) {
        next_lv_map->erase(del_itr);
    }

    // if next level is the bottom, delete all "~DELETED~" flags
    bool is_delete = overflowed_index == disk_levels.size() - 2;
    auto merged = merge_table(overflowed_tables, is_delete, level_path);

    for (auto insert : merged) {
        next_level->push_back(insert);
    }

}

bool DiskRepo::check_overflow(size_t index) {
    if (index >= disk_levels.size()) return true;
    return disk_levels[index]->get_size() <= (size_t)(1 << (index + 1));
}

void DiskRepo::push_ssTable(SSTable *new_table) {
    disk_levels[0]->push_back(new_table);
    size_t cur_level = 0;
    while (!check_overflow(cur_level)) {
        // overflow -> compaction
        handle_overflow(cur_level);
        cur_level++;
    }
}

void DiskRepo::push_ssTable(ListNode *head, uint64_t kv_count) {
    if (!kv_count) return;

    if (disk_levels.empty()) {
        create_level(0);
    }

    auto new_ssTable = new SSTable(head, kv_count, time_stamp++, dir + "/level-0");
    push_ssTable(new_ssTable);
}

void DiskRepo::push_table(SkipList *memTable) {
    ListNode *head = memTable->get_bottom_head();
    uint64_t kv_count = memTable->get_kv_count();
    push_ssTable(head, kv_count);
}

std::string DiskRepo::get(uint64_t key) {
    std::string ret_string;
    uint64_t max_time_stamp = 0;
    for (auto cur_level : disk_levels) {
        uint64_t cur_ts = 0;
        std::string cur_str = cur_level->get(key, cur_ts);
        if (cur_ts > max_time_stamp) {
            ret_string = cur_str;
            max_time_stamp = cur_ts;
        }
    }
    return ret_string == "~DELETED~" ? "" : ret_string;
}

void DiskRepo::clear() {
    for (auto del_level : disk_levels) {
        del_level->delete_level();
    }
    disk_levels.clear();
    time_stamp = 1;
}

bool DiskRepo::check_overlap() {
    auto level = disk_levels.begin() + 1;
    while (level != disk_levels.end()) {
        if (!(*level)->check_overlap())
            return false;
        level++;
    }
    return true;
}