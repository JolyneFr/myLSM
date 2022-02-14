#include "kvstore.h"
#include <string>


KVStore::KVStore(const std::string &dir):
    KVStoreAPI(dir), diskStore(DiskRepo(dir)) {}

KVStore::~KVStore() {
    diskStore.push_table(&memTable);
}

void KVStore::put(uint64_t key, const std::string &s)
{
    if (!memTable.put(key, s)) {
        diskStore.push_table(&memTable);
        memTable.clear();
        memTable.put(key, s);
    }
}

std::string KVStore::get(uint64_t key)
{
	std::string mem_str = memTable.get(key);
	if (!mem_str.empty()) {
	    return mem_str == "~DELETED~" ? "" : mem_str;
	} return diskStore.get(key);
}

bool KVStore::del(uint64_t key)
{
    bool is_exist = !get(key).empty();
	if (is_exist) {
        memTable.put(key, "~DELETED~");
	} return is_exist;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
    memTable.clear();
    diskStore.clear();
}
