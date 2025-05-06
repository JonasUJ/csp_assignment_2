#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <cstdint>
#include <utility>
#include <vector>

class BPlusTree {
public:
    explicit BPlusTree(int degree);
    void insert(uint32_t key, uint32_t value);
    uint32_t query(uint32_t key) const;
    void display() const;
    std::vector<std::pair<uint32_t, uint32_t>> range(uint32_t low, uint32_t high) const;

private:
    class Node;
    Node *root;
    int degree;
    void insertInternal(uint32_t key, Node *parent, Node *child);
    void splitInternal(Node *node);
    void splitLeaf(Node *leaf);
    static Node *findParent(Node *current, Node *child);
};

#endif
