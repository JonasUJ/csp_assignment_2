#include "b_plus_tree.h"
#include "trie.h"
#include <iostream>

int main() {
  Trie trie;
  trie.node.value = 0;
  trie.node.key = 0;

  insert(&trie, 2, 2);
  insert(&trie, 4, 4);
  insert(&trie, 8, 8);
  insert(&trie, 8 << 16, 8 << 16);
  insert(&trie, 8 << 24, 8 << 24);

  uint32_t val = query(&trie, 8);
  std::cout << "query(8) = " << val << std::endl;

  std::cout << "Range query from 0 to (8 << 16) + 1:\n";
  auto results = range(&trie, 3, (8 << 16) + 1);
  for (auto &[k, v] : results) {
    std::cout << "key: " << k << ", value: " << v << "\n";
  }

  std::cout << "Full Trie:\n";
  print(&trie.node);

  int degree = 3;
  BPlusTree tree(degree);
  tree.insert(10);
  tree.insert(20);
  tree.insert(5);
  tree.insert(6);
  tree.insert(12);
  tree.insert(30);
  tree.insert(7);
  tree.insert(17);

  tree.display();
  std::cout << "B+ Tree range query";
  std::vector<int> b_range = tree.range(10, 25);
  for (auto v : b_range) {
    std::cout << v << " ";
  }

  return 0;
}
