#pragma once

#include "global.h"

class MergeBuffer {
private:

    ListNode *head, *rear;
    uint64_t data_count = 0;  // number of data in buffer
    uint64_t data_total_length = 0; // total length of all strings

    void delete_all();

public:
    // Construct an empty buffer.
    MergeBuffer();

    // Destruct a MergeBuffer, delete all nodes included.
    ~MergeBuffer();

    // Add a key-value pair to buffer with sorted sequence.
    bool push_back(uint64_t key, const std::string& value);

    // Head pointer of linked list.
    ListNode *get_head();

    // Rear pointer of linked list.
    ListNode *get_rear();

    // Number of kv-pair stored in linked list.
    uint64_t get_size() const;

    // Clear the buffer.
    void clear();

};