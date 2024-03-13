#ifndef __NODE__
#define __NODE__

#include "json.hpp"
#include "utils.h"
#include "cmp.h"
#include<seal/seal.h>
using namespace seal;

using json = nlohmann::json;
using namespace std;

typedef unsigned Leaf;

class Node {
public:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    int feature_index;
    
    uint64_t threshold;
    vector<uint64_t> threshold_bitv;
    vector<Plaintext> threshold_bitv_plain;
    
    uint64_t class_leaf; 
    
    std::string op;

    Ciphertext value;

    Node() = default;
    explicit Node(json::reference j);
    explicit Node(std::string filename);
    json to_json();
    json to_json_with_random_value(int bit_length);
    void gen_with_depth(int d);
    unsigned get_depth();
    bool is_leaf() const;
    uint64_t eval(const std::vector<uint64_t> &features);
    int leaf_num();
    void tecmp_pdte_init(int l,int m);
    void cdcmp_pdte_init(seal::BatchEncoder *batch_encoder,int num_cmps, int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row);
    void rdcmp_pdte_init(seal::BatchEncoder *batch_encoder,int n, int num_cmps, uint64_t slot_count,uint64_t row_count);
};

void print_node(Node& a, std::string tabs = "");

void print_tree(Node& a);

void build_tree_from_json(json::reference ref, Node& node);
void build_json_from_tree(Node& node, json::reference ref);

std::vector<std::vector<uint64_t>> read_csv_to_vector(std::string address,int data_m);

void leaf_extract_rec(vector<uint64_t>& out,Node& node );





#endif