#include "DiskManager.h"

void DiskManager::handle_overflow(size_t overflowed_index) {
    LevelStorage *upper_level = disk_levels[overflowed_index];
    std::vector<SSTable*> overflowed_tables;

    if (overflowed_index == 0) {
        // level-0: compact all
        overflowed_tables = (*upper_level->get_level());
        disk_levels[overflowed_index] = new LevelStorage(overflowed_index);
    } else {
        // not level-0
        size_t merge_num = disk_levels[overflowed_index]->get_size() - (1 << (overflowed_index + 1));

    }
}

bool DiskManager::check_overflow(size_t index) {
    return disk_levels[index]->get_size() <= (1 << (index + 1));
} 