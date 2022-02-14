#include "SSTable.h"
#include <map>

class Level {

private:

    std::map<key_type, SSTable*> level_tables;

    std::string level_path;

public:

    /**
     * Construct a level from its directory and level-number.
     * Directory must exist before calling this function.
     */
    Level(const std::string& level_dir, size_t ls);

    /**
     * Scan all existing SSTable in level directory.
     * Called just after construction.
     * @return max time_stamp of SSTable in this Level
     */
    uint64_t scan_level();

    /**
     * Destruct a level, leaving its data on disk.
     */
    ~Level();

    /**
     * Add a new SSTable into level, sorted by time stamp.
     * @param new_ssTable constructed & saved new SSTable
     */
    void push_back(SSTable *new_ssTable);

    /**
     * @return number of SSTables in the level
     */
    size_t get_size();

    /**
     * Pop out k SSTables with smallest time_stamp & min_key.
     * @return vector of popped SSTables
     */
    std::vector<SSTable*> pop_k(size_t k);

    /**
     * Search a string (include "~DELETED~") by its key.
     * ret_ts would be set to time_stamp of SSTable containing this kv-pair.
     */
    std::string get(uint64_t key, uint64_t &ret_ts);

    /**
     * Delete directory linked with this Level.
     */
    void delete_level();

    /**
     * @return Linked directory path of this Level.
     */
    std::string get_level_path();

    /**
     * @return pointer to storage map of Level
     */
    std::map<key_type, SSTable*> *get_level();

    bool check_overlap();
};