#include <algorithm>
#include <iostream>
#include <vector>

#include "b_plus_tree.h"

using namespace std;

// const int DEGREE = 3; // degree (t) = Minimum number of children for a node
// max keys = 2*t - 1

class BPlusTree::Node {
public:
    bool isLeaf;
    std::vector<uint32_t> keys;
    std::vector<uint32_t> values;
    std::vector<Node *> children;
    Node *next; // for leaves

    explicit Node(const bool leaf) : isLeaf(leaf), next(nullptr) {}
};

BPlusTree::BPlusTree(int degree) : degree(degree), root(nullptr) {}

void BPlusTree::insertInternal(uint32_t key, Node *parent, Node *child) {
    if (!parent) {
        root = new Node(false);
        root->keys.push_back(key);
        root->children.push_back(child);
        root->children.push_back(nullptr);
        return;
    }

    const auto it = ranges::upper_bound(parent->keys, key);
    const uint32_t idx = it - parent->keys.begin();
    parent->keys.insert(it, key);
    parent->children.insert(parent->children.begin() + idx + 1, child);

    if (parent->keys.size() >= 2 * degree) {
        splitInternal(parent);
    }
}

void BPlusTree::splitInternal(Node *node) {
    const int midIndex = node->keys.size() / 2;
    const uint32_t midKey = node->keys[midIndex];

    Node *sibling = new Node(false);
    sibling->keys.assign(node->keys.begin() + midIndex + 1, node->keys.end());
    sibling->children.assign(node->children.begin() + midIndex + 1, node->children.end());

    node->keys.resize(midIndex);
    node->children.resize(midIndex + 1);

    if (node == root) {
        const auto newRoot = new Node(false);
        newRoot->keys.push_back(midKey);
        newRoot->children.push_back(node);
        newRoot->children.push_back(sibling);
        root = newRoot;
    } else {
        insertInternal(midKey, findParent(root, node), sibling);
    }
}

void BPlusTree::splitLeaf(Node *leaf) {
    const uint32_t midIndex = leaf->keys.size() / 2;

    const auto sibling = new Node(true);
    sibling->keys.assign(leaf->keys.begin() + midIndex, leaf->keys.end());
    sibling->values.assign(leaf->values.begin() + midIndex, leaf->values.end());

    leaf->keys.resize(midIndex);
    leaf->values.resize(midIndex);

    sibling->next = leaf->next;
    leaf->next = sibling;

    if (leaf == root) {
        const auto newRoot = new Node(false);
        newRoot->keys.push_back(sibling->keys[0]);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(sibling);
        root = newRoot;
    } else {
        insertInternal(sibling->keys[0], findParent(root, leaf), sibling);
    }
}

BPlusTree::Node *BPlusTree::findParent(Node *current, Node *child) {
    if (current->isLeaf)
        return nullptr;

    for (const auto c: current->children) {
        if (c == child)
            return current;
        if (Node *p = findParent(c, child))
            return p;
    }
    return nullptr;
}

void BPlusTree::insert(const uint32_t key, const uint32_t value) {
    if (!root) {
        root = new Node(true);
        root->keys.push_back(key);
        root->values.push_back(value);
        return;
    }

    Node *current = root;
    while (!current->isLeaf) {
        auto it = ranges::upper_bound(current->keys, key);
        const uint32_t idx = it - current->keys.begin();
        current = current->children[idx];
    }

    // Find position for insertion
    const auto it = ranges::lower_bound(current->keys, key);
    const uint32_t idx = it - current->keys.begin();

    // Update value if key already exists
    if (it != current->keys.end() && *it == key) {
        current->values[idx] = value;
        return;
    }

    // Insert new key-value pair
    current->keys.insert(it, key);
    current->values.insert(current->values.begin() + idx, value);

    if (current->keys.size() >= 2 * degree) {
        splitLeaf(current);
    }
}

uint32_t BPlusTree::query(const uint32_t key) const {
    if (!root)
        return 0;

    Node *current = root;
    while (!current->isLeaf) {
        auto it = ranges::upper_bound(current->keys, key);
        const uint32_t idx = it - current->keys.begin();
        current = current->children[idx];
    }

    const auto it = ranges::lower_bound(current->keys, key);
    const uint32_t idx = it - current->keys.begin();

    if (it != current->keys.end() && *it == key) {
        return current->values[idx];
    }

    return 0; // Return 0 if key not found
}

void BPlusTree::display() const {
    if (!root)
        return;
    const Node *current = root;
    while (!current->isLeaf)
        current = current->children[0];

    while (current) {
        for (size_t i = 0; i < current->keys.size(); ++i) {
            std::cout << "(" << current->keys[i] << ":" << current->values[i] << ") ";
        }
        std::cout << "| ";
        current = current->next;
    }
    std::cout << "\n";
}

std::vector<std::pair<uint32_t, uint32_t>> BPlusTree::range(const uint32_t low, const uint32_t high) const {
    std::vector<std::pair<uint32_t, uint32_t>> result;

    if (!root)
        return result;

    Node *current = root;
    while (!current->isLeaf) {
        auto it = ranges::upper_bound(current->keys, low);
        const uint32_t idx = it - current->keys.begin();
        current = current->children[idx];
    }

    while (current) {
        for (size_t i = 0; i < current->keys.size(); ++i) {
            uint32_t key = current->keys[i];
            if (key > high)
                return result;
            if (key >= low) {
                result.push_back(std::make_pair(key, current->values[i]));
            }
        }
        current = current->next;
    }

    return result;
}
