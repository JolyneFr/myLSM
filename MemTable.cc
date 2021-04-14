#include "MemTable.h"
#include <vector>
#include <random>
#include <iostream>

MemTable::ListNode::ListNode(): 
    prev(nullptr), next(nullptr), below(nullptr) {}

MemTable::ListNode::ListNode(const uint64_t &k, const std::string &v):
    key(k), value(v), prev(nullptr), next(nullptr), below(nullptr) {}

MemTable::ListNode::~ListNode() {
    if (!prev && below) delete below; 
    if (next) delete next;
    value.~basic_string();
}

MemTable::MemTable() {
    head = new ListNode();
}

MemTable::~MemTable() {
    delete head;
}

uint64_t MemTable::cal_size(uint64_t count, uint64_t length) {
    return 10272 + 12 * count + length;
}

MemTable::ListNode *MemTable::ListNode::insertAfterAbove(ListNode *p, ListNode *b) {
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

MemTable::ListNode *MemTable::find(uint64_t key) {
    ListNode *target = head;
    while (target) {
        while (target->next && target->next->key < key) {
            target = target->next;
        }
        if (target->next && target->next->key == key) {
            return target->next;
        }
        target = target->below;
    }
    return nullptr;
}

std::string MemTable::get(uint64_t key) {
    ListNode *find_node = find(key);
    if (find_node) {
        return find_node->value;
    } else {
        return "";
    }
}

bool MemTable::put(uint64_t key, std::string value) {
    std::vector<ListNode*> path_list;
    ListNode *hot = head;
    while (hot) {
        while (hot->next && hot->next->key < key) {
            hot = hot->next;
        }
        if (hot->next && hot->next->key == key) {
            // handle coverage
            hot = hot->next;
            size_t old_length = hot->value.size();
            uint64_t pred_length = data_total_length + value.size() - old_length;
            if (cal_size(data_count, pred_length) > MAX_BYTE_SIZE) {
                return false;
            }
            while (hot) {
                hot->value = value;
                hot = hot->below;
            }
            data_total_length = pred_length;
            return true;
        }
        path_list.push_back(hot);
        hot = hot->below;
    }

    // handle insert
    bool isUp = true;
    ListNode *below_node = nullptr;
    while (isUp && path_list.size() > 0) {
        ListNode *prev_node = path_list.back();
        path_list.pop_back();
        ListNode *new_node = new ListNode(key, value);
        new_node->insertAfterAbove(prev_node, below_node);
        below_node = new_node;
        isUp = (rand() & 1);
    }
    if (isUp) {
        ListNode *old_head = head;
        head = new ListNode();
        ListNode *new_node = new ListNode(key, value);
        new_node->insertAfterAbove(head, below_node);
        head->below = old_head;
    }
    uint64_t pred_length = data_total_length + value.size();
    if (cal_size(data_count + 1, pred_length) > MAX_BYTE_SIZE) {
        return false;
    }
    data_count++;
    data_total_length = pred_length;
    return true;
}

bool MemTable::remove(uint64_t key) {
    ListNode *top_target = find(key);
    if (!top_target) return false;
    uint64_t str_length = top_target->value.size();
    while (top_target) {
        ListNode *next = top_target->next;
        ListNode *prev = top_target->prev;
        if (next) next->prev = prev;
        if (prev) prev->next = next;
        top_target = top_target->below;
    }
    data_count--;
    data_total_length -= str_length;
    return true;
}

uint64_t MemTable::mem_size() {
    return cal_size(data_count, data_total_length);
}

void MemTable::clear() {
    delete head;
    head = new ListNode();
}

void MemTable::show() {
    ListNode *cur_layer = head;
    while (cur_layer) {
        ListNode *cur_data = cur_layer->next;
        while (cur_data) {
            std::cout << cur_data->key << " " << cur_data->value << " | ";
            cur_data = cur_data->next;
        }
        std::cout << std::endl;
        cur_layer = cur_layer->below;
    }
    std::cout << "Memory Usage: " << mem_size() << " Bytes.\n";
}