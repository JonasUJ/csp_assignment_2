#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include <vector>

class BPlusTree {
public:
  BPlusTree(int degree);
  void insert(int key);
  bool search(int key);
  void display();
  std::vector<int> range(int low, int high);

private:
  class Node;
  Node *root;
  int degree;
  void insertInternal(int key, Node *parent, Node *child);
  void splitInternal(Node *node);
  void splitLeaf(Node *leaf);
  Node *findParent(Node *current, Node *child);
};

#endif
