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
  std::vector<int> keys;
  std::vector<Node *> children;
  Node *next;  // for leaves

  Node(bool leaf) : isLeaf(leaf), next(nullptr) {}
};

BPlusTree::BPlusTree(int degree) : degree(degree), root(nullptr) {}

void BPlusTree::insertInternal(int key, Node *parent, Node *child) {
  if (!parent) {
    root = new Node(false);
    root->keys.push_back(key);
    root->children.push_back(child);
    root->children.push_back(nullptr);
    return;
  }

  auto it = std::upper_bound(parent->keys.begin(), parent->keys.end(), key);
  int idx = it - parent->keys.begin();
  parent->keys.insert(it, key);
  parent->children.insert(parent->children.begin() + idx + 1, child);

  if (parent->keys.size() >= 2 * degree) {
    splitInternal(parent);
  }
}

void BPlusTree::splitInternal(Node *node) {
  int midIndex = node->keys.size() / 2;
  int midKey = node->keys[midIndex];

  Node *sibling = new Node(false);
  sibling->keys.assign(node->keys.begin() + midIndex + 1, node->keys.end());
  sibling->children.assign(node->children.begin() + midIndex + 1,
                           node->children.end());

  node->keys.resize(midIndex);
  node->children.resize(midIndex + 1);

  if (node == root) {
    Node *newRoot = new Node(false);
    newRoot->keys.push_back(midKey);
    newRoot->children.push_back(node);
    newRoot->children.push_back(sibling);
    root = newRoot;
  } else {
    insertInternal(midKey, findParent(root, node), sibling);
  }
}

void BPlusTree::splitLeaf(Node *leaf) {
  int midIndex = leaf->keys.size() / 2;

  Node *sibling = new Node(true);
  sibling->keys.assign(leaf->keys.begin() + midIndex, leaf->keys.end());

  leaf->keys.resize(midIndex);

  sibling->next = leaf->next;
  leaf->next = sibling;

  if (leaf == root) {
    Node *newRoot = new Node(false);
    newRoot->keys.push_back(sibling->keys[0]);
    newRoot->children.push_back(leaf);
    newRoot->children.push_back(sibling);
    root = newRoot;
  } else {
    insertInternal(sibling->keys[0], findParent(root, leaf), sibling);
  }
}

BPlusTree::Node *BPlusTree::findParent(Node *current, Node *child) {
  if (current->isLeaf) return nullptr;

  for (auto c : current->children) {
    if (c == child) return current;
    Node *p = findParent(c, child);
    if (p) return p;
  }
  return nullptr;
}

void BPlusTree::insert(int key) {
  if (!root) {
    root = new Node(true);
    root->keys.push_back(key);
    return;
  }

  Node *current = root;
  while (!current->isLeaf) {
    auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
    int idx = it - current->keys.begin();
    current = current->children[idx];
  }

  auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
  current->keys.insert(it, key);

  if (current->keys.size() >= 2 * degree) {
    splitLeaf(current);
  }
}

bool BPlusTree::search(int key) {
  if (!root) return false;

  Node *current = root;
  while (!current->isLeaf) {
    auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
    int idx = it - current->keys.begin();
    current = current->children[idx];
  }

  return std::find(current->keys.begin(), current->keys.end(), key) !=
         current->keys.end();
}

void BPlusTree::display() {
  if (!root) return;
  Node *current = root;
  while (!current->isLeaf) current = current->children[0];

  while (current) {
    for (auto k : current->keys) std::cout << k << " ";
    std::cout << "| ";
    current = current->next;
  }
  std::cout << "\n";
}

std::vector<int> BPlusTree::range(int low, int high) {
  std::vector<int> result;

  if (!root) return result;

  Node *current = root;
  while (!current->isLeaf) {
    auto it = std::upper_bound(current->keys.begin(), current->keys.end(), low);
    int idx = it - current->keys.begin();
    current = current->children[idx];
  }

  while (current) {
    for (int key : current->keys) {
      if (key > high) return result;
      if (key >= low) result.push_back(key);
    }
    current = current->next;
  }

  return result;
}
