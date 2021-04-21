#include "kvstore.h"
#include <string>


KVStore::KVStore(const std::string &dir):
    KVStoreAPI(dir), store(DiskManager(dir)), data_dir(dir) {}

KVStore::~KVStore() = default;

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
    if (!memTable.put(key, s)) {
        uint64_t kv_count = memTable.get_kv_count();
        uint64_t ts = store.get_time_stamp();
        auto new_ssTable = new SSTable(memTable.get_bottom_head(), kv_count, ts, data_dir + "/level-0/");
        store.push_ssTable(new_ssTable);

        memTable.clear();
        memTable.put(key, s);
    }
}

/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	std::string mem_str = memTable.get(key);
	if (!mem_str.empty()) {
	    if (mem_str == "~DELETED~")
	        return "";
	    return mem_str;
	} return store.get(key);
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
    bool is_exist = !get(key).empty();
	memTable.put(key, "~DELETED~");
    return is_exist;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    memTable.clear();
    store.clear();
}
