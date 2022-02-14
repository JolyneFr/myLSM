#include "MergeBuffer.h"

MergeBuffer::MergeBuffer() {
    head = new ListNode;
    rear = head;
}

MergeBuffer::~MergeBuffer() {
    delete_all();
}

bool MergeBuffer::push_back(uint64_t key, const std::string& value) {
    uint64_t pred_length = data_total_length + value.size();
    if (cal_size(data_count + 1, pred_length) > MAX_BYTE_SIZE)
        return false;
    auto new_node = new ListNode(key, value);
    new_node->insertAfterAbove(rear, nullptr);
    rear = new_node;

    data_count++;
    data_total_length = pred_length;
    return true;
}

ListNode *MergeBuffer::get_head() {
    return head;
}

ListNode *MergeBuffer::get_rear() {
    return rear;
}

uint64_t MergeBuffer::get_size() const {
    return data_count;
}

void MergeBuffer::clear() {
    delete_all();
    head = new ListNode;
    rear = head;
}

void MergeBuffer::delete_all() {
    ListNode *cur_node = head;
    while (cur_node) {
        ListNode *del_node = cur_node;
        cur_node = cur_node->next;
        delete del_node;
    }
    data_count = 0;
    data_total_length = 0;
}