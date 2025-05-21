#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <random>
#include <thread>
#include <atomic>
#include <ranges>
#include "b_plus_tree.h"
#include "trie.h"


const int inserts_for_each_size = 500000;
const int num_queries = 1000000;

template<typename Structure, typename QueryFn>
int run_parallel_queries(Structure &data_structure, QueryFn query_fn, int num_queries, int num_threads,
                         std::function<uint32_t()> key_gen) {
    std::vector<uint32_t> keys(num_queries);

    // Generate all keys up front
    for (int i = 0; i < num_queries; ++i) {
        keys[i] = key_gen();
    }

    std::vector<std::thread> threads;
    int queries_per_thread = num_queries / num_threads;
    std::atomic<bool> start_flag(false);

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            int start_idx = t * queries_per_thread;
            int end_idx = (t + 1) * queries_per_thread;

            // Wait until the start flag is set
            while (!start_flag.load(std::memory_order_acquire));

            for (int i = start_idx; i < end_idx; ++i) {
                uint32_t key = keys[i];
                volatile auto val = query_fn(data_structure, key);
            }
        });
    }

    auto start = std::chrono::high_resolution_clock::now();
    start_flag.store(true, std::memory_order_release);

    for (auto &thread: threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    return static_cast<int>(diff.count());
}

void measure_random_queries_tries(std::vector<Trie> &tries, const std::string &label, const std::vector<int> &threads,
                                  std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[" << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < tries.size(); i++) {
            Trie &trie = tries[i];

            std::mt19937 rng(static_cast<unsigned>(i + thread_count));
            std::uniform_int_distribution<uint32_t> dist(0, static_cast<uint32_t>((i + 1) * inserts_for_each_size - 1));

            auto time_sec = run_parallel_queries(
                    trie, [](Trie &t, uint32_t key) { return query(&t, key); }, num_queries, thread_count,
                    [&]() { return dist(rng); });


            std::cout << label << " Trie size " << (i + 1) * inserts_for_each_size << ": " << time_sec << "ns\n";
            out << label << "," << (i + 1) * inserts_for_each_size << ",random," << thread_count << "," << time_sec
                << "\n";
        }
    }
}


void measure_skewed_queries_tries(std::vector<Trie> &tries, const std::string &label, const std::vector<int> &threads,
                                  std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[Skewed Query Test: " << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < tries.size(); i++) {
            Trie &trie = tries[i];
            std::mt19937 rng(static_cast<unsigned>(i + 999 + thread_count));
            std::exponential_distribution<> skew(0.00001);

            int max_key = (i + 1) * inserts_for_each_size;

            auto key_gen = [=]() mutable { return static_cast<uint32_t>(skew(rng)) % max_key; };

            auto time_sec = run_parallel_queries(
                    trie, [](Trie &t, uint32_t key) -> uint32_t { return query(&t, key); }, num_queries, thread_count,
                    key_gen);

            std::cout << label << " Trie size " << (i + 1) * inserts_for_each_size << ": " << time_sec << "ns\n";
            out << label << "," << (i + 1) * inserts_for_each_size << ",skewed," << thread_count << "," << time_sec
                << "\n";
        }
    }
}

void measure_random_queries_bplus(const std::vector<BPlusTree> &trees, const std::string &label,
                                  const std::vector<int> &threads, std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[" << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < trees.size(); i++) {
            const BPlusTree &tree = trees[i];

            std::mt19937 rng(static_cast<unsigned>(i + thread_count));
            std::uniform_int_distribution<uint32_t> dist(0, static_cast<uint32_t>((i + 1) * inserts_for_each_size - 1));

            auto time_sec = run_parallel_queries(
                    tree, [](const BPlusTree &t, uint32_t key) { return t.query(key); }, num_queries, thread_count,
                    [&]() { return dist(rng); });


            std::cout << label << " B+Tree size " << (i + 1) * inserts_for_each_size << ": " << time_sec << "ns\n";
            out << label << "," << (i + 1) * inserts_for_each_size << ",random," << thread_count << "," << time_sec
                << "\n";
        }
    }
}

void measure_skewed_queries_bplus(const std::vector<BPlusTree> &trees, const std::string &label,
                                  const std::vector<int> &threads, std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[Skewed Query Test: " << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < trees.size(); i++) {
            const BPlusTree &tree = trees[i];
            std::mt19937 rng(static_cast<unsigned>(i + 999 + thread_count));
            std::exponential_distribution<> skew(0.00001);

            int max_key = (i + 1) * inserts_for_each_size;

            auto key_gen = [=]() mutable { return static_cast<uint32_t>(skew(rng)) % max_key; };

            auto time_sec = run_parallel_queries(
                    tree, [](const BPlusTree &t, uint32_t key) -> uint32_t { return t.query(key); }, num_queries,
                    thread_count, key_gen);

            std::cout << label << " B+Tree size " << (i + 1) * inserts_for_each_size << ": " << time_sec << "ns\n";
            out << label << "," << (i + 1) * inserts_for_each_size << ",skewed," << thread_count << "," << time_sec
                << "\n";
        }
    }
}


int main(int argc, char *argv[]) {
    std::vector<int> threads = {1, 2, 4, 8, 16, 32};
    if (argc >= 2) {

        std::vector<Trie> dense_tries;
        std::vector<Trie> sparse_tries;
        std::vector<BPlusTree> dense_bp_trees;
        std::vector<BPlusTree> sparse_bp_trees;
        int number_of_sizes = 10;
        int bp_degree = 64; // so each node fits in a cache line of 64 bytes. Each key is 4 bytes

        for (int i = 1; i <= number_of_sizes; i++) {
            Trie trie;
            trie.node.value = 0;
            trie.node.key = 0;
            BPlusTree tree(bp_degree);
            int number_of_inserts = i * inserts_for_each_size;
            for (int j = 0; j < number_of_inserts; j++) {
                insert(&trie, j, j);
                tree.insert(j, j);
            }

            dense_tries.push_back(trie);
            dense_bp_trees.push_back(tree);
            std::cout << "Created one trie and tree of size " << number_of_inserts << std::endl;
        }

        std::cout << "All tries created" << std::endl;

        for (int i = 1; i <= number_of_sizes; i++) {
            Trie trie;
            trie.node.value = 0;
            trie.node.key = 0;
            BPlusTree tree(bp_degree);

            int max_key_size = i * inserts_for_each_size * 2;

            std::mt19937 rng(i);
            std::uniform_int_distribution<uint32_t> dist(0, max_key_size - 1);

            for (int j = 0; j < number_of_sizes; j++) {
                uint32_t key = dist(rng);
                insert(&trie, key, key);
            }

            sparse_tries.push_back(trie);
            sparse_bp_trees.push_back(tree);
        }

        std::cout << "All trees created" << std::endl;

        std::ofstream out("results.csv", std::ios::app);
        if (out.tellp() == 0) {
            out << "structure,size,query_type,thread_level,time\n";
        }

        measure_random_queries_tries(dense_tries, "Dense Trie", threads, out);
        measure_random_queries_tries(sparse_tries, "Sparse Trie", threads, out);

        measure_skewed_queries_tries(dense_tries, "Dense Trie (skew)", threads, out);
        measure_skewed_queries_tries(sparse_tries, "Sparse Trie (skew)", threads, out);


        measure_random_queries_bplus(dense_bp_trees, "Dense B+ Tree", threads, out);
        measure_random_queries_bplus(sparse_bp_trees, "Sparse B+ Tree", threads, out);

        measure_skewed_queries_bplus(dense_bp_trees, "Dense B+ Tree (skew)", threads, out);
        measure_skewed_queries_bplus(sparse_bp_trees, "Sparse B+ Tree (skew)", threads, out);

        out.close();

    } else {
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
        for (auto &[k, v]: trie_results) {
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


        std::cout << "Adding random values to B+ Tree:\n";
        for (int i = 0; i < 10; ++i) {
            // Add some random values
            std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
            std::uniform_int_distribution<uint32_t> dist(1, (i + 1) * inserts_for_each_size);
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
        for (auto &[k, v]: b_tree_results) {
            std::cout << "key: " << k << ", value: " << v << "\n";
        }
    }
    return 0;
}
