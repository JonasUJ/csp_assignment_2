#include "b_plus_tree.h"
#include "trie.h"
#include <iostream>
#include <random>
#include <ctime>

int main() {
  // Test Trie
  std::cout << "=== Testing Trie ===\n";
  Trie trie;
  trie.node.value = 0;
  trie.node.key = 0;

  insert(&trie, 2, 2);
  insert(&trie, 4, 4);
  insert(&trie, 8, 8);
  insert(&trie, 8 << 16, 8 << 16);
  insert(&trie, 8 << 24, 8 << 24);

  uint32_t val = query(&trie, 8);
  std::cout << "Trie query(8) = " << val << std::endl;

  std::cout << "Trie range query from 3 to (8 << 16) + 1:\n";
  auto trie_results = range(&trie, 3, (8 << 16) + 1);
  for (auto &[k, v] : trie_results) {
    std::cout << "key: " << k << ", value: " << v << "\n";
  }

  std::cout << "Full Trie:\n";
  print(&trie.node);

  // Test B+ Tree
  std::cout << "\n=== Testing B+ Tree ===\n";
  int degree = 3;
  BPlusTree tree(degree);
  
  // Insert the same values as trie for comparison
  tree.insert(2, 2);
  tree.insert(4, 4);
  tree.insert(8, 8);
  tree.insert(8 << 16, 8 << 16);
  tree.insert(8 << 24, 8 << 24);
  
  // Add some random values
  std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
  std::uniform_int_distribution<uint32_t> dist(1, 1000000);
  
  std::cout << "Adding random values to B+ Tree:\n";
  for (int i = 0; i < 10; ++i) {
    uint32_t random_key = dist(rng);
    uint32_t random_value = dist(rng);
    std::cout << "Adding key: " << random_key << ", value: " << random_value << "\n";
    tree.insert(random_key, random_value);
  }

  // Test query
  val = tree.query(8);
  std::cout << "B+ Tree query(8) = " << val << "\n";

  // Display B+ Tree
  std::cout << "B+ Tree contents:\n";
  tree.display();
  
  // Test range query
  std::cout << "B+ Tree range query from 3 to (8 << 16) + 1:\n";
  auto b_tree_results = tree.range(3, (8 << 16) + 1);
  for (auto &[k, v] : b_tree_results) {
    std::cout << "key: " << k << ", value: " << v << "\n";
  }

  return 0;
}
