#include "node.h"
#include <iostream>
#include <fstream>


void print_node(Node& a, string tabs){
    if (a.is_leaf()){
       cout << tabs << "(class: " << a.class_leaf << ")" << endl; 
    }else{
        cout << tabs
            << "(f: " << a.feature_index 
            << ", t: " << a.threshold
            << ")" << endl; 
    }
}

void print(std::shared_ptr<Node> a, string tabs) {
    if (a->is_leaf()){
       print_node(*a, tabs);
    }else{
        print(a->right, tabs + "        ");
        print_node(*a, tabs);
        print(a->left, tabs + "        ");
    }
}

void print_tree(Node& a) {
    std::shared_ptr<Node> tmp_a = std::make_shared<Node>(a);
    print(tmp_a, " ");
}

void build_tree_from_json(json::reference ref, Node& node) {
    if (!ref["leaf"].is_null()) {
        node.class_leaf = ref["leaf"].get<Leaf>();
        node.left = nullptr;
        node.right = nullptr;
    } else {
        node.threshold = ref["internal"]["threshold"].get<uint64_t>();
        node.feature_index = ref["internal"]["feature"].get<unsigned>();
        node.op = ref["internal"]["op"].get<std::string>();
        node.class_leaf = -1;

        node.left = std::make_shared<Node>();
        node.right = std::make_shared<Node>();
        build_tree_from_json(ref["internal"]["left"], *node.left);
        build_tree_from_json(ref["internal"]["right"], *node.right);
    }
}

Node::Node(json::reference j) {
    build_tree_from_json(j, *this);
}

Node::Node(std::string filename) {
    std::ifstream file(filename);
    nlohmann::json ref = nlohmann::json::parse(file);
    build_tree_from_json(ref, *this);
}


// Convert Node structure to JSON object
    json Node::to_json(){
        json j;
        if(left == nullptr && right == nullptr){
            j["leaf"] = this->class_leaf;
        }else{
            j["internal"]["threshold"] = this->threshold;
            j["internal"]["feature"] = this->feature_index;
            j["internal"]["op"] = this->op;
            j["internal"]["left"] = this->left->to_json();
            j["internal"]["right"] = this->right->to_json();
        }
        return j;
    }


// Convert Node structure to JSON object
    json Node::to_json_with_random_value(int bit_length){
        json j;
        if(left == nullptr && right == nullptr){
            j["leaf"] = this->class_leaf;
        }else{
            j["internal"]["threshold"] = rand()% (1<<bit_length);
            j["internal"]["feature"] = this->feature_index;
            j["internal"]["op"] = this->op;
            j["internal"]["left"] = this->left->to_json_with_random_value(bit_length);
            j["internal"]["right"] = this->right->to_json_with_random_value(bit_length);
        }
        return j;
    }

bool Node::is_leaf() const {
    return this->left == nullptr && this->right == nullptr;
}

void Node::gen_with_depth(int d) {
    if (d == 0) {
        this->class_leaf = 0;
        this->left = nullptr;
        this->right = nullptr;
        return;
    }
    this->class_leaf = -1;
    (*this->left).gen_with_depth(d-1);
    (*this->right).gen_with_depth(d-1);
}

unsigned Node::get_depth() {
    if (this->is_leaf()) {
        return 0;
    } else {
        auto l = this->left->get_depth();
        auto r = this->right->get_depth();
        if (l > r) {
            return l + 1;
        } else {
            return r + 1;
        }
    }
}

void eval_rec(uint64_t &out, const Node& node, const std::vector<uint64_t> &features, u_int64_t parent) {
    if (node.is_leaf()) {
        //cout<<"node.class_leaf = " <<node.class_leaf<< " parent = "<<parent<<endl;
        out += node.class_leaf * parent;
    }else{
        if (node.op == "leq") {
            //cout<<"features[" <<node.feature_index<< "] = "<<features[node.feature_index] <<" > "<< node.threshold <<"  --- "<<(features[node.feature_index] > node.threshold)<<endl;
            if (features[node.feature_index] >= node.threshold) {
                eval_rec(out, *node.left, features, parent*(1-parent));
                eval_rec(out, *node.right, features, parent);
                
            } else {
                eval_rec(out, *node.left, features, parent);
                eval_rec(out, *node.right, features, parent*(1-parent));
            }
        } else {
            // unimplemented
            assert(false);
        }
    }
}

uint64_t Node::eval(const std::vector<uint64_t> &features) {
    uint64_t out = 0;
    uint64_t parent = 1;
    eval_rec(out, *this, features, parent);
    return out;
}


void leaf_num_rec(Node& a, int &num){
    if (a.is_leaf()){
        num++;
    }else{
        leaf_num_rec(*(a.left), num);
        leaf_num_rec(*(a.right), num);
    }
}

void cdcmp_pdte_init_rec(Node& a, seal::BatchEncoder *batch_encoder,int num_cmps, int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row){
    if (a.is_leaf()){
    }else{
        vector<uint64_t> plain_op;
        for(int i = 0; i < num_cmps ;i++){
            plain_op.push_back( a.threshold);
        }

        //we do not use threshold == 0, then return ciphertext zero,
        //because threshold == 0 is no mean in practical way, 
        //we want to test the full tree process. 
        //plain_op[num_cmps-1] = 4097; maybe error if data_m == num_cmps, but just test.
        if(a.threshold==0){
            plain_op[num_cmps-1] = 4097;
        }

        Plaintext server_input;
        vector<uint64_t> plain_op_encode = cdcmp_encode_a(plain_op,num_slots_per_element,slot_count,row_count,num_cmps_per_row);
        batch_encoder->encode(plain_op_encode, server_input);
        a.threshold_bitv_plain.push_back(server_input);
        cdcmp_pdte_init_rec(*(a.left), batch_encoder, num_cmps, num_slots_per_element, slot_count,row_count, num_cmps_per_row);
        cdcmp_pdte_init_rec(*(a.right), batch_encoder, num_cmps, num_slots_per_element, slot_count,row_count, num_cmps_per_row);
    }
}


void Node::cdcmp_pdte_init(seal::BatchEncoder *batch_encoder,int num_cmps, int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row) {
    cdcmp_pdte_init_rec(*this, batch_encoder, num_cmps, num_slots_per_element, slot_count,row_count, num_cmps_per_row);
}

void tecmp_pdte_init_rec(Node& a, int l,int m){
    if (a.is_leaf()){

    }else{
        vector<uint64_t> plain_op(l, 0ULL);
        for(int i = 0 ; i < l; i++){
            plain_op[ i ] = (a.threshold >> (i * m) ) & ((1 << m) - 1);
        }
        a.threshold_bitv = plain_op;
        tecmp_pdte_init_rec(*(a.left), l, m);
        tecmp_pdte_init_rec(*(a.right), l, m);
    }
}

void Node::tecmp_pdte_init(int l,int m) {
    tecmp_pdte_init_rec(*this, l, m);
}

void rdcmp_pdte_init_rec(Node& a, seal::BatchEncoder *batch_encoder,int n, int num_cmps, uint64_t slot_count, uint64_t row_count){
    if (a.is_leaf()){
    }else{
        vector<uint64_t> plain_op;
        vector<Plaintext> server_input(n);
        for(int i = 0; i < num_cmps ;i++){
            plain_op.push_back( a.threshold);
        }
        vector<vector<uint64_t>> plain_op_encode = rdcmp_encode_a(plain_op,n,slot_count,row_count);
        for(int i = 0 ; i < n; i++){
            batch_encoder->encode(plain_op_encode[i], server_input[i]);
        }
        a.threshold_bitv_plain = server_input;
        rdcmp_pdte_init_rec(*(a.left), batch_encoder,n, num_cmps, slot_count,row_count);
        rdcmp_pdte_init_rec(*(a.right), batch_encoder,n, num_cmps, slot_count,row_count);
    }
}

void Node::rdcmp_pdte_init(seal::BatchEncoder *batch_encoder,int n, int num_cmps, uint64_t slot_count,uint64_t row_count ) {
    rdcmp_pdte_init_rec(*this, batch_encoder, n, num_cmps,slot_count,row_count);
}

/**/
int Node::leaf_num() {
    int num = 0;
    leaf_num_rec(*this, num);
    return num;
}

void build_json_from_tree(Node& node, json::reference ref){
    if(node.is_leaf()){
        ref["leaf"]=node.class_leaf;
    }else{
        ref["internal"]["threshold"]=node.threshold;
        ref["internal"]["feature"]=node.feature_index;
        ref["internal"]["op"]=node.op;
        build_json_from_tree(*node.left, ref["internal"]["left"]);
        build_json_from_tree(*node.right, ref["internal"]["right"]);
    }
}

std::vector<std::vector<uint64_t>> read_csv_to_vector(std::string address,int data_m){
    std::vector<std::vector<uint64_t>> data;
    std::ifstream file(address);

    if (!file.is_open()) {
        std::cerr << "open files error" << std::endl;
        return data;
    }
    std::string line;

    // read csv line by line
    int i=0;
    while(i<data_m && std::getline(file, line)){
        std::vector<uint64_t> row;
        std::stringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ',')) {
            uint64_t cellValue;
            std::istringstream(cell) >> cellValue;
            row.push_back(cellValue);
        }

        data.push_back(row);
        i++;
    }
    return data;
}

void leaf_extract_rec(vector<uint64_t>& out,Node& node ){
    if (node.is_leaf()){
        out.push_back(node.class_leaf);
    }else{
        leaf_extract_rec(out, *(node.left));
        leaf_extract_rec(out, *(node.right));
    }
}