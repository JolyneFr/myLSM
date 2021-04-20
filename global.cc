#include "global.h"

uint64_t cal_size(uint64_t count, uint64_t length) {
    return 10272 + 12 * count + length;
}

std::string my_itoa(uint64_t tmp) {
    std::string tmp_string;
    std::stringstream buf;
    buf << tmp;
    buf >> tmp_string;
    return tmp_string;
}

void ListNode::insertAfterAbove(ListNode *p, ListNode *b){
    // above node doesn't exists
    ListNode *n = nullptr;
    if (p) {
        n = p->next;
        p->next = this;
    }
    if (n) { n->prev = next; }
    this->prev = p;
    this->next = n;
    this->below = b;
}

bool isInScope(std::pair<uint64_t, uint64_t> scope, uint64_t key) {
    return key >= scope.first && key <= scope.second;
}

bool operator<(MergeData a, MergeData b) {
    if (a.min_key == b.min_key)
        // smaller time stamp -> later popped
        return a.time_stamp < b.time_stamp;
    // smaller key -> earlier popped
    return a.min_key > b.min_key;
}