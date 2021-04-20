#include <queue>
#include "LevelStorage.h"

LevelStorage::LevelStorage(size_t l): level(l) {}

LevelStorage::~LevelStorage() = default;

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
        if (cur_scope.first > key) right = mid - 1;
        else left = mid + 1;
    }
    return std::make_pair(left, right);
}

std::pair<std::vector<SSTable*>::iterator, std::vector<SSTable*>::iterator> LevelStorage::find_overlap(scope_type scope) {
    scope_type left_scope = binary_search(scope.first);
    scope_type right_scope = binary_search(scope.first);

    // the overlapped scope is [begin, end]
    auto begin = level_tables.begin() + left_scope.first;
    auto end = level_tables.begin() + right_scope.second;

    return std::make_pair(begin, end);
}

size_t LevelStorage::get_size() {
    return level_tables.size();
}

std::vector<SSTable*> *LevelStorage::get_level() {
    return &level_tables;
}

std::vector<SSTable*> LevelStorage::get_min_k(size_t k) {
    std::priority_queue<SSTable> select_heap;
//    for (SSTable *cur_table : level_tables) {
//        select_heap.push(*cur_table);
//    }
//    for (size_t i = 0; i < k; i++) {
//
//    }
}