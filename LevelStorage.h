#include "SSTable.h"

class LevelStorage {

private:

    std::vector<SSTable*> level_tables;

    size_t level;

    std::string level_path;

    /**
     * search for the position of given key
     * @param key query key
     * @return pair<left, right>, which indicates:
     *         if left == right, key is in the scope of index left (except 0, need special judge)
     *         if left + 1 == right, key is between the scopes of index left and right
     */
    scope_type binary_search(uint64_t key);

public:

    LevelStorage(const std::string& level_dir, size_t ls);
    ~LevelStorage();

    void push_back(SSTable *);

    std::pair<std::vector<SSTable*>::iterator, std::vector<SSTable*>::iterator> find_overlap(std::pair<uint64_t, uint64_t>);

    std::vector<SSTable*> *get_level();

    size_t get_size();

    std::vector<SSTable*> pop_k(size_t k);

    void set_tables(std::vector<SSTable*> init_tables);

    std::string get(uint64_t key, uint64_t &ret_ts);

    void delete_level();

    std::string get_level_path();

    uint64_t scan_level();

};