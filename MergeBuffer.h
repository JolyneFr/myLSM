#pragma once

#include "global.h"

class MergeBuffer {
private:

    ListNode *head, *rear;
    uint64_t data_count = 0;  // number of datas in buffer
    uint64_t data_total_length = 0; // total length of all strings

public:

    MergeBuffer();

    ~MergeBuffer();

    bool push_back(uint64_t key, std::string value);

    ListNode *get_head();

    ListNode *get_rear();

    uint64_t get_size();

    void pop_back();

    void clear();

    void delete_all();
};