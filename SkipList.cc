#include "SkipList.h"
#include <vector>
#include <random>
#include <iostream>

SkipList::SkipList() {
    head = new ListNode();
    lowest_head = head;
}

SkipList::~SkipList() {
    ListNode *del_head = head;
    while (del_head) {
        ListNode *cur_node = del_head;
        del_head = del_head->below;
        while (cur_node) {
            ListNode *del_node = cur_node;
            cur_node = cur_node->next;
            delete del_node;
        }
    }
}

ListNode *SkipList::find(uint64_t key) {
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

std::string SkipList::get(uint64_t key) {
    ListNode *find_node = find(key);
    if (find_node) {
        return find_node->value;
    } else {
        return "";
    }
}

bool SkipList::put(uint64_t key, std::string value) {
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

bool SkipList::remove(uint64_t key) {

    ListNode *top_target = find(key);
    if (!top_target) return false;
    uint64_t str_length = top_target->value.size();
    while (top_target) {
        ListNode *next = top_target->next;
        ListNode *prev = top_target->prev;
        ListNode *del_node = top_target;
        top_target = top_target->below;
        if (next) next->prev = prev;
        if (prev) prev->next = next;
        delete del_node;
    }
    data_count--;
    data_total_length -= str_length;
    return true;

}

std::vector<value_type> *SkipList::exported_data() {
    std::vector<value_type> *data_set = new std::vector<value_type>();
    ListNode *cur_data = head;
    while (cur_data->below) {
        cur_data = cur_data->below; // find the lowest layer
    }
    cur_data = cur_data->next;
    while (cur_data) {
        data_set->push_back(std::make_pair(cur_data->key, cur_data->value));
        cur_data = cur_data->next;
    }
    return data_set;
}

ListNode *SkipList::get_bottom_head() {
    return lowest_head;
}

uint64_t SkipList::mem_size() {
    return cal_size(data_count, data_total_length);
}

uint64_t SkipList::get_kv_count() {
    return data_count;
}

void SkipList::clear() {
    this->~SkipList();
    head = new ListNode();
    lowest_head = head;
    data_count = 0;
    data_total_length = 0;
}

void SkipList::show() {
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