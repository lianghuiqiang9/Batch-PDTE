#ifndef __UTILS__H
#define __UTILS__H

#include "seal/seal.h"
#include "string"
#include <cassert>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;
constexpr uint64_t DomainExtensionModulo = 4096;

class Timer {
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;

    public:
    Timer();
    void start();
    void end();
    long double end_and_get();
    void reset();
    long double get_time_in_milliseconds();
};

struct Metrics {
    map<string, uint64_t> metrics_;
};

using Config = struct Config;

struct Config {
    bool debug_mode = false;
    bool verbose = false;
    uint64_t bitlength = 0;
    string write_to_file = "results/";
    bool mult_path = false;
};

struct Node {
    uint64_t threshold;
    uint64_t attr_idx;
    seal::Ciphertext ctxt;
    Node *left = nullptr;
    Node *right = nullptr;

    Node(uint64_t threshold, uint64_t attr_idx, Node *left, Node *right);
    inline bool is_leaf() const {
        return (left == nullptr);
    }
    bool is_leaf2() const;
};

inline string uint64_to_hex_string(uint64_t value) {
    return seal::util::uint_to_hex_string(&value, size_t(1));
}

void read_attr_vec(const string &fname, vector<uint64_t> &dest); 

int mod_inverse(int a, int m);

uint64_t rand_uint64();

Node *build_tree_from_json(json::reference ref);
Node *Node2(std::string filename);
void print_node(Node& a, std::string tabs = "");

void print_tree(Node* a);
#endif
