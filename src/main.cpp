#include <unistd.h>
#include <cassert>
#include <cstring>
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
const int num_runs = 5;
const double skew_degree = 0.00001;
const int window_size = 100;


template<typename Structure, typename QueryFn>
long run_parallel_queries(Structure &data_structure, QueryFn query_fn, int num_queries, int num_threads,
                         std::function<uint32_t()> key_gen) {
    long total_time = 0;

    for (int run = 0; run < num_runs; ++run) {
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
        total_time += static_cast<long>(diff.count());
    }

    return total_time / num_runs; // Return the mean time
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
            std::exponential_distribution<> skew(skew_degree);

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

void measure_range_queries_tries(std::vector<Trie> &tries, const std::string &label, const std::vector<int> &threads,
                                 std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[Range Query Test: " << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < tries.size(); i++) {
            Trie &trie = tries[i];
            int max_key = (i + 1) * inserts_for_each_size;

            std::mt19937 rng(static_cast<unsigned>(i + thread_count));
            std::uniform_int_distribution<uint32_t> dist(0, max_key - 1);

            auto time_sec = run_parallel_queries(
                trie,
                [&](Trie &t, uint32_t key) -> uint32_t {
                    uint32_t low = key;
                    uint32_t high = std::min(key + window_size, static_cast<uint32_t>(max_key));
                    return range(&t, low, high).size();
                },
                num_queries,
                thread_count,
                [&]() { return dist(rng); }
            );

            std::cout << label << " Trie size " << max_key << ": " << time_sec << "ns\n";
            out << label << "," << max_key << ",range," << thread_count << "," << time_sec << "\n";
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
            std::exponential_distribution<> skew(skew_degree);

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

void measure_range_queries_bplus(const std::vector<BPlusTree> &trees, const std::string &label, const std::vector<int> &threads,
                                 std::ofstream &out) {
    for (auto thread_count: threads) {
        std::cout << "\n[Range Query Test: " << label << "] Thread level: " << thread_count << "\n";

        for (size_t i = 0; i < trees.size(); i++) {
            const BPlusTree &tree = trees[i];
            int max_key = (i + 1) * inserts_for_each_size;

            std::mt19937 rng(static_cast<unsigned>(i + thread_count));
            std::uniform_int_distribution<uint32_t> dist(0, max_key - 1);

            auto time_sec = run_parallel_queries(
                tree,
                [&](const BPlusTree &t, uint32_t key) -> uint32_t {
                    uint32_t low = key;
                    uint32_t high = std::min(key + window_size, static_cast<uint32_t>(max_key));
                    return t.range(low, high).size();
                },
                num_queries,
                thread_count,
                [&]() { return dist(rng); }
            );

            std::cout << label << " B+ Tree size " << max_key << ": " << time_sec << "ns\n";
            out << label << "," << max_key << ",range," << thread_count << "," << time_sec << "\n";
        }
    }
}

void start_perf(int perf_ctl_fd, int perf_ctl_ack_fd) {
    write(perf_ctl_fd, "enable\n", 7);
    char ack[5] = {};
    read(perf_ctl_ack_fd, ack, 5);
    assert(strcmp(ack, "ack\n") == 0);
}

void stop_perf(int perf_ctl_fd, int perf_ctl_ack_fd) {
    write(perf_ctl_fd, "disable\n", 8);
    char ack[5] = {};
    read(perf_ctl_ack_fd, ack, 5);
    assert(strcmp(ack, "ack\n") == 0);
}

int main(int argc, char *argv[]) {
    std::vector<int> threads = {1, 2, 4, 8, 16, 32};

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
        std::cout << "Created one dense trie and tree of size " << number_of_inserts << std::endl;
    }

    for (int i = 1; i <= number_of_sizes; i++) {
        Trie trie;
        trie.node.value = 0;
        trie.node.key = 0;
        BPlusTree tree(bp_degree);

        int max_key_size = i * inserts_for_each_size * 2;

        std::mt19937 rng(i);
        std::uniform_int_distribution<uint32_t> dist(0, max_key_size - 1);

        for (int j = 0; j < max_key_size / 2; j++) {
            uint32_t key = dist(rng);
            insert(&trie, key, key);
            tree.insert(key, key);
        }

        sparse_tries.push_back(trie);
        sparse_bp_trees.push_back(tree);
        std::cout << "Created one sparse trie and tree of size " << max_key_size / 2 << std::endl;
    }

    std::cout << "All tries/trees created" << std::endl;

    std::ofstream out("results.csv", std::ios::app);
    if (out.tellp() == 0) {
        out << "structure,size,query_type,thread_level,time\n";
    }

    measure_random_queries_tries(dense_tries, "Dense Trie", threads, out);
    measure_random_queries_tries(sparse_tries, "Sparse Trie", threads, out);

    measure_skewed_queries_tries(dense_tries, "Dense Trie (skew)", threads, out);
    measure_skewed_queries_tries(sparse_tries, "Sparse Trie (skew)", threads, out);

    measure_range_queries_tries(dense_tries, "Dense Trie (range)", threads, out);

    measure_random_queries_bplus(dense_bp_trees, "Dense B+ Tree", threads, out);
    measure_random_queries_bplus(sparse_bp_trees, "Sparse B+ Tree", threads, out);

    measure_skewed_queries_bplus(dense_bp_trees, "Dense B+ Tree (skew)", threads, out);
    measure_skewed_queries_bplus(sparse_bp_trees, "Sparse B+ Tree (skew)", threads, out);

    measure_range_queries_bplus(dense_bp_trees, "Dense B+ Tree (range)", threads, out);

    out.close();

    return 0;
}
