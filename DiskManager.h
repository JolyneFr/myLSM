#pragma once

#include "LevelStorage.h"
#include "SkipList.h"

class DiskManager {

private:

    std::vector<LevelStorage*> disk_levels;

    void handle_overflow(size_t overflowed_index);

    bool check_overflow(size_t index);

    uint64_t time_stamp;

    const std::string dir;

    void push_ssTable(SSTable *new_table);

    void push_ssTable(ListNode *head, uint64_t kv_count);

public:

    explicit DiskManager(const std::string& dir);

    ~DiskManager();

    void push_table(SkipList *memTable);

    std::string get(uint64_t key);

    void clear();

};

