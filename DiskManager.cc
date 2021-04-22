#include "DiskManager.h"
#include "utils.h"

DiskManager::DiskManager(const std::string& d): time_stamp(1), dir(d) {
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
                if (cur_match_name == (*dir_str)) {
                    auto new_level = new LevelStorage(dir, i);
                    uint64_t max_ts = new_level->scan_level();
                    if (!new_level->get_size()) { // no sstable file exists
                        dir_str = dir_list.end();
                        delete new_level;
                        break;
                    }
                    if (time_stamp <= max_ts) time_stamp = max_ts + 1;
                    disk_levels.push_back(new_level);
                }
                dir_str++;
            }
            if (dir_str == dir_list.end()) break;
        }
    }
}

DiskManager::~DiskManager() {
    for (auto level : disk_levels) {
        delete level;
    }
}

void DiskManager::handle_overflow(size_t overflowed_index) {
    LevelStorage *upper_level = disk_levels[overflowed_index];
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
        std::string new_level_dir = dir + "/level-" + my_itoa(overflowed_index + 1);
        utils::mkdir(new_level_dir.c_str());
        auto new_level = new LevelStorage(dir, overflowed_index + 1);
        disk_levels.push_back(new_level);
    }

    LevelStorage *next_level = disk_levels[overflowed_index + 1];
    // merge a table from upper level at once => maybe faster
    for (SSTable *cur_table : overflowed_tables) {
        std::vector<SSTable*> merge_set = { cur_table };
        auto overlap_boundary = next_level->find_overlap(cur_table->getScope());
        auto level_path = next_level->get_level_path();

        auto level_data = next_level->get_level();
        auto header = std::vector<SSTable*>(level_data->begin(), overlap_boundary.first);
        auto overlap = std::vector<SSTable*>(overlap_boundary.first, overlap_boundary.second);
        auto tail = std::vector<SSTable*>(overlap_boundary.second, level_data->end());

        overlap.push_back(cur_table);
        // if next level is the bottom, delete all "~DELETED~" flags
        bool is_delete = overflowed_index == disk_levels.size() - 2;
        std::vector<SSTable*> merged_set = merge_table(overlap, time_stamp, is_delete, level_path);

        std::vector<SSTable*> new_level_data;
        for (SSTable *append_table : header) {
            new_level_data.push_back((append_table));
        }
        for (SSTable *append_table : merged_set) {
            new_level_data.push_back((append_table));
        }
        for (SSTable *append_table : tail) {
            new_level_data.push_back((append_table));
        }
        next_level->set_tables(new_level_data);
    }

}

bool DiskManager::check_overflow(size_t index) {
    if (index >= disk_levels.size()) return true;
    return disk_levels[index]->get_size() <= (1 << (index + 1));
}

void DiskManager::push_ssTable(SSTable *new_table) {
    if (disk_levels.empty()) {
        std::string lv0 = dir + "/level-0";
        utils::mkdir(lv0.c_str());
        disk_levels.push_back(new LevelStorage(dir, 0));
    }
    disk_levels[0]->push_back(new_table);
    size_t cur_level = 0;
    while (!check_overflow(cur_level)) {
        // overflow -> compaction
        handle_overflow(cur_level);
        cur_level++;
    }
}

void DiskManager::push_ssTable(ListNode *head, uint64_t kv_count) {
    if (!kv_count) return;

    auto new_ssTable = new SSTable(head, kv_count, time_stamp++, dir + "/level-0");
    push_ssTable(new_ssTable);
}

void DiskManager::push_table(SkipList *memTable) {
    ListNode *head = memTable->get_bottom_head();
    uint64_t kv_count = memTable->get_kv_count();
    push_ssTable(head, kv_count);
}

std::string DiskManager::get(uint64_t key) {
    std::string ret_string;
    uint64_t max_time_stamp = 0;
    for (auto cur_level : disk_levels) {
        uint64_t cur_ts = 0;
        std::string cur_str = cur_level->get(key, cur_ts);
        if (cur_ts > max_time_stamp && cur_str != "~DELETED~") {
            ret_string = cur_str;
            max_time_stamp = cur_ts;
        }
    }
    return ret_string;
}

void DiskManager::clear() {
    for (auto del_level : disk_levels) {
        del_level->delete_level();
    }
}