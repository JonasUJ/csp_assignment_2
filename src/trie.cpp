#include "trie.h"
#include <cstdio>
#include <unordered_map>

static Node *find(Node *node, const uint8_t key) {
    auto it = node->nodes.find(key);
    if (it != node->nodes.end()) {
        return &(it->second);
    }
    return nullptr;
}

void insert(Trie *trie, const uint32_t key, const uint32_t value) {
    Node *node = &trie->node;
    for (int shift = 24; shift >= 0; shift -= 8) {
        constexpr uint32_t mask = 0xff;
        const auto masked_key = static_cast<uint8_t>((key >> shift) & mask);

        auto [it, inserted] = node->nodes.emplace(masked_key, Node{masked_key, 0, {}});
        node = &(it->second);
    }
    node->value = value;
}

uint32_t query(Trie *trie, const uint32_t key) {
    Node *node = &trie->node;
    for (int shift = 24; shift >= 0; shift -= 8) {
        constexpr uint32_t mask = 0xff;
        const auto masked_key = static_cast<uint8_t>((key >> shift) & mask);
        Node *res = find(node, masked_key);

        if (res == nullptr) {
            return 0;
        }
        node = res;
    }
    return node->value;
}

static void rangeHelper(Node *node, const int depth, uint32_t key, const uint32_t low, const uint32_t high,
                        std::vector<std::pair<uint32_t, uint32_t>> &result) {
    if (depth == 4) {
        if (key >= low && key <= high && node->value != 0) {
            result.emplace_back(key, node->value);
        }
        return;
    }

    for (auto &kv: node->nodes) {
        const auto &child = kv.second;
        const uint32_t shift = 24 - depth * 8;
        const uint32_t new_key = key | (static_cast<uint32_t>(child.key) << shift);

        if (new_key <= high && (new_key | ((1u << (24 - depth * 8)) - 1)) >= low) {
            rangeHelper(const_cast<Node*>(&child), depth + 1, new_key, low, high, result);
        }
    }
}

std::vector<std::pair<uint32_t, uint32_t>> range(Trie *trie, const uint32_t low, const uint32_t high) {
    std::vector<std::pair<uint32_t, uint32_t>> result;
    rangeHelper(&trie->node, 0, 0, low, high, result);
    return result;
}

void print(Node *node, int level) {
    printf("%*sk:%u, v:%u, n:%zu\n", level * 2, "", node->key, node->value, node->nodes.size());
    for (auto &kv: node->nodes) {
        print(&kv.second, level + 1);
    }
}
