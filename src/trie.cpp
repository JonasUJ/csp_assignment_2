#include "trie.h"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <vector>

struct Node {
	std::vector<Node> nodes;
	int value;
	char key;
};

struct Trie {
	Node node;
};

Trie create() {
	return Trie{Node{std::vector<Node>(), 0, 0}};
}

Node* find(Node* node, char key) {
	for (int i = 0; i < node->nodes.size(); i++) {
		auto other = &node->nodes.at(i);
		if (other->key == key) {
			return other;
		}
	}
	return nullptr;
}

void insert(Trie* trie, int key, int value) {
	const int mask = 0xff;

	auto node = &trie->node;
	for (int shift = 0; shift < 32; shift += 8) {
		char masked_key = (char)((mask << shift & key) >> shift);
		auto res = find(node, masked_key);

		if (res == nullptr) {
			Node new_node{std::vector<Node>(), 0, masked_key};
			node->nodes.push_back(new_node);
			node = &node->nodes.at(node->nodes.size() - 1);
		} else {
			node = res;
		}
	}

	node->value = value;
}

int query(Trie* trie, int key) {
	const int mask = 0xff;

	auto node = &trie->node;
	for (int shift = 0; shift < 32; shift += 8) {
		char masked_key = (char)((mask << shift & key) >> shift);
		auto res = find(node, masked_key);

		if (res == nullptr) {
			return 0;
		} else {
			node = res;
		}
	}

	return node->value;
}

void print(Node* node, int level) {
	printf("%*sk:%i,v:%i,n:%zu\n", level*2, "", node->key, node->value, node->nodes.size());
	for (int i = 0; i < node->nodes.size(); i++) {
		print(&node->nodes.at(i), level + 1);
	}
}

void helloWorldTrie() {
	auto trie = create();
	insert(&trie, 8, 8);
	insert(&trie, 8<<16, 8<<16);
	auto val = query(&trie, 8);
	std::cout << "value: " << val << std::endl;
	print(&trie.node, 0);
}
