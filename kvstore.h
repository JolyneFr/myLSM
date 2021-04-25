#pragma once

#include "kvstore_api.h"
#include "SkipList.h"
#include "DiskRepo.h"

class KVStore : public KVStoreAPI {
	// You can add your implementation here
private:
    SkipList memTable;
    DiskRepo diskStore;

public:
    /**
     * Construct a KVStore under "dir".
     * Load data into memory if exists.
     * @param dir root directory path of new KVStore
     */
	explicit KVStore(const std::string &dir);

	/**
	 * Destruct KVStore after writing data in memTable to disk.
	 */
	~KVStore();

	/**
	 * Insert/Update the key-value pair.
     * No return values for simplicity.
	 */
	void put(uint64_t key, const std::string &s) override;

    /**
    * Returns the (string) value of the given key.
    * An empty string indicates not found.
    */
	std::string get(uint64_t key) override;

    /**
     * Delete the given key-value pair if it exists.
     * Returns false iff the key is not found.
     */
	bool del(uint64_t key) override;

    /**
     * This resets the kvstore. All key-value pairs should be removed,
     * including memtable and all sstables files.
     */
	void reset() override;

};
