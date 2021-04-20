#include "SSTable.h"

class LevelStorage {

private:

    std::vector<SSTable*> level_tables;

    size_t level;

    scope_type binary_search(uint64_t key);

public:

    LevelStorage(size_t ls);
    ~LevelStorage();

    void push_back(SSTable *);

    std::pair<std::vector<SSTable*>::iterator, std::vector<SSTable*>::iterator> find_overlap(std::pair<uint64_t, uint64_t>);

    std::vector<SSTable*> *get_level();

    size_t get_size();

    std::vector<SSTable*> get_min_k(size_t k);

};