#include "SSTable.h"
#include <map>

class LevelStorage {

private:

    std::map<key_type, SSTable*> level_tables;

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

    size_t get_size();

    std::vector<SSTable*> pop_k(size_t k);

    std::string get(uint64_t key, uint64_t &ret_ts);

    void delete_level();

    std::string get_level_path();

    uint64_t scan_level();

    std::map<key_type, SSTable*> *get_level();

};