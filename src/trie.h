#ifndef TRIE_H
#define TRIE_H

#include <cstdint>
#include <utility>
#include <vector>

struct Node {
    std::vector<Node> nodes;
    uint32_t value;
    uint8_t key;
};

struct Trie {
    Node node;
};

void insert(Trie *trie, uint32_t key, uint32_t value);
uint32_t query(Trie *trie, uint32_t key);
std::vector<std::pair<uint32_t, uint32_t>> range(Trie *trie, uint32_t low, uint32_t high);

void print(Node *node, int level = 0);

#endif // TRIE_H
