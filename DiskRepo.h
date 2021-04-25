#pragma once

#include "Level.h"
#include "SkipList.h"

class DiskRepo {

private:

    std::vector<Level*> disk_levels;

    uint64_t time_stamp;

    const std::string dir;

    void handle_overflow(size_t overflowed_index);

    bool check_overflow(size_t index);

    void push_ssTable(SSTable *new_table);

    void push_ssTable(ListNode *head, uint64_t kv_count);

    void create_level(uint64_t ls);

public:

    /**
     * Construct a level-structured disk repository for SSTables.
     */
    explicit DiskRepo(const std::string& dir);

    /**
     * Destruct the disk repository (no file or directory deleted).
     */
    ~DiskRepo();

    /**
     * Add a new SStable into repo, and handle possible overflow.
     * @param memTable SkipList storing data to be inserted.
     */
    void push_table(SkipList *memTable);

    /**
    * Returns the (string) value of the given key in disk.
    * An empty string indicates not found.
    */
    std::string get(uint64_t key);

    /**
     * Delete all Levels and SSTables in the root directory.
     * Reset time_stamp to 1.
     */
    void clear();

};

