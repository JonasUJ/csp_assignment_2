#ifndef TRIE_H
#define TRIE_H

#include <cstdint>
#include <utility>
#include <vector>
#include <unordered_map>

struct Node {
    uint8_t key;
    uint32_t value;
    std::unordered_map<uint8_t, Node> nodes;
};

struct Trie {
    Node node;
};

void insert(Trie *trie, uint32_t key, uint32_t value);
uint32_t query(Trie *trie, uint32_t key);
std::vector<std::pair<uint32_t, uint32_t>> range(Trie *trie, uint32_t low, uint32_t high);

void print(Node *node, int level = 0);

#endif // TRIE_H
