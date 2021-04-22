#include <queue>
#include "LevelStorage.h"
#include "utils.h"

LevelStorage::LevelStorage(const std::string& dir, size_t l): level(l) {
    level_path = dir + "/level-" + my_itoa(l);
}

uint64_t LevelStorage::scan_level() {
    std::vector<std::string> dir_list;
    utils::scanDir(level_path, dir_list);
    uint64_t max_ts = 0;

    // heap sort ->
    std::priority_queue<KeyIndexPair> sort_heap;
    size_t table_index = 0;
    std::vector<SSTable*> sort_buffer;
    for (const auto& file_str : dir_list) {
        if (sst_suffix(file_str.c_str())) {
            auto new_ssTable = new SSTable(level_path + "/" + file_str);
            if (new_ssTable->get_time_stamp() > max_ts) {
                max_ts = new_ssTable->get_time_stamp();
            }
            sort_buffer.push_back(new_ssTable);
            sort_heap.push(KeyIndexPair(new_ssTable->getScope().first, table_index++));
        }
    }
    while (!sort_heap.empty()) {
        level_tables.push_back(sort_buffer[sort_heap.top().index]);
        sort_heap.pop();
    }
    return max_ts;
}

LevelStorage::~LevelStorage() {
    for (SSTable *del_table : level_tables) {
        delete del_table;
    }
}

void LevelStorage::push_back(SSTable* new_ssTable) {
    level_tables.push_back(new_ssTable);
}

scope_type LevelStorage::binary_search(uint64_t key) {
    size_t left = 0, right = level_tables.size() - 1;
    while (left <= right) {
        size_t mid = (left + right) >> 1;
        auto cur_scope = level_tables[mid]->getScope();
        if (isInScope(cur_scope, key))
            return std::make_pair(mid, mid);
        if (cur_scope.first > key) {
            if (mid > 0) right = mid - 1;
            else return std::make_pair(0, 0);
        } else left = mid + 1;
    }
    return std::make_pair(right, left);
}

std::pair<std::vector<SSTable*>::iterator, std::vector<SSTable*>::iterator> LevelStorage::find_overlap(scope_type scope) {

    if (level_tables.empty()) {
        return std::make_pair(level_tables.end(), level_tables.end());
    }

    scope_type left_scope = binary_search(scope.first);
    scope_type right_scope = binary_search(scope.second);

    // the overlapped scope is [begin, end)
    auto begin = level_tables.begin() + left_scope.second;
    auto end = level_tables.begin() + right_scope.first + 1;
    if (right_scope.first == 0 && right_scope.second == 0 &&
    !isInScope(level_tables[0]->getScope(), scope.second)) { // less than min key
        end = level_tables.begin();
    }

    return std::make_pair(begin, end);
}

size_t LevelStorage::get_size() {
    return level_tables.size();
}

std::vector<SSTable*> *LevelStorage::get_level() {
    return &level_tables;
}

std::vector<SSTable*> LevelStorage::pop_k(size_t k) {

    std::vector<SSTable*> selected_tables;
    if (k == level_tables.size()) { // pop all
        selected_tables = level_tables;
        level_tables.clear();
    } else { // pop tables with smaller time stamp
        std::priority_queue<TimeIndexPair> select_heap;
        size_t level_size = level_tables.size();
        for (size_t i = 0; i < level_size; ++i) {
            select_heap.push(TimeIndexPair(level_tables[i]->get_time_stamp(), i));
        }
        for (size_t i = 0; i < k; ++i) {
            SSTable *new_table = level_tables[select_heap.top().index];
            select_heap.pop();
            selected_tables.push_back(new_table);
        }
    }
    return selected_tables;
}

void LevelStorage::set_tables(std::vector<SSTable*> init_tables) {
    level_tables = std::move(init_tables);
}

std::string LevelStorage::get(uint64_t key, uint64_t &ret_ts) {
    if (level == 0) {
        // level-0 -> can overlap, check every table
        std::string ret_string;
        for (auto check_table : level_tables) {
            if (isInScope(check_table->getScope(), key)) {
                std::string buf_string = check_table->get(key);
                uint64_t buf_ts = check_table->get_time_stamp();
                if (!buf_string.empty() && buf_ts > ret_ts) {
                    // hit! change the status
                    ret_ts = buf_ts;
                    ret_string = buf_string;
                }
            }
        }
        return ret_string;
    }
    scope_type key_scope = binary_search(key);
    SSTable *target_table = level_tables[key_scope.first];
    if (isInScope(target_table->getScope(), key)) {
        std::string ret_string = target_table->get(key);
        if (!ret_string.empty()) {
            ret_ts = target_table->get_time_stamp();
            return ret_string;
        }
    }
    // not in any scope
    return "";
}

void LevelStorage::delete_level() {
    for (auto del_table : level_tables) {
        del_table->delete_file();
        utils::rmdir(level_path.c_str());
    }
}

std::string LevelStorage::get_level_path() {
    return level_path;
}