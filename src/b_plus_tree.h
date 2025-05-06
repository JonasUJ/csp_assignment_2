#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <cstdint>
#include <utility>
#include <vector>

class BPlusTree {
public:
    BPlusTree(int degree);
    void insert(uint32_t key, uint32_t value);
    uint32_t query(uint32_t key);
    void display();
    std::vector<std::pair<uint32_t, uint32_t>> range(uint32_t low, uint32_t high);

private:
    class Node;
    Node *root;
    int degree;
    void insertInternal(uint32_t key, Node *parent, Node *child);
    void splitInternal(Node *node);
    void splitLeaf(Node *leaf);
    Node *findParent(Node *current, Node *child);
};

#endif
