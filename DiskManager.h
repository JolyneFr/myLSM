#pragma once

#include "LevelStorage.h"

class DiskManager {

private:

    std::vector<LevelStorage*> disk_levels;

    void handle_overflow(size_t overflowed_index);

    bool check_overflow(size_t index);

    uint64_t time_stamp;

    const std::string dir;

public:

    explicit DiskManager(const std::string& dir);

    ~DiskManager();

    void push_ssTable(SSTable *new_table);

    std::string get(uint64_t key);

    uint64_t get_time_stamp() const;

    void clear();

};

