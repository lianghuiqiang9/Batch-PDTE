#include "tree_utils.h"





bool DecisionTreeNode::is_leaf2() const {
    return this->left_child == NULL && this->right_child == NULL;
}


void print_node(DecisionTreeNode& a, string tabs){
    if (a.is_leaf2()){
       cout << tabs << "(class: " << a.threshold_value << ")" << endl; 
    }else{
        cout << tabs
            << "(f: " << a.attribute_used 
            << ", t: " << a.threshold_value
            << ")" << endl; 
    }
}

void print(DecisionTreeNode* a, string tabs) {
    if (a->is_leaf2()){
       print_node(*a, tabs);
    }else{
        print(a->right_child, tabs + "        ");
        print_node(*a, tabs);
        print(a->left_child, tabs + "        ");
    }
}

void print_tree(DecisionTreeNode* a) {
    //std::shared_ptr<Node> tmp_a = std::make_shared<Node>(a);
    auto temp_a = a;
    print(temp_a, " ");
}


DecisionTreeNode *build_tree_from_json(json::reference ref) {

    DecisionTreeNode *ret = nullptr;

    if (!ref["leaf"].is_null()) {
        uint64_t threshold = ref["leaf"].get<uint64_t>();
        ret = new DecisionTreeNode(threshold, -1);
    } else {

        uint64_t threshold = ref["internal"]["threshold"].get<uint64_t>();
        uint64_t attr_idx = ref["internal"]["feature"].get<uint64_t>();
        //node.op = ref["internal"]["op"].get<std::string>();
        //node.class_leaf = -1;
        auto left = build_tree_from_json(ref["internal"]["left"]);
        auto right = build_tree_from_json(ref["internal"]["right"]);
        ret = new DecisionTreeNode(threshold, attr_idx);
        ret->left_child = left;
        ret->right_child = right;

    }

    return ret;
}


DecisionTreeNode * Node2(std::string filename) {
    std::ifstream file(filename);
    nlohmann::json ref = nlohmann::json::parse(file);
    auto ret = build_tree_from_json(ref);
    return ret;
}




QueryParameters::QueryParameters(
    int n, int num_attr, int hamming_weight,
    uint64_t num_threads, PathEvaluation path_eval, ComparisonType comparison, string write_to_file
){
    this->k = hamming_weight;
    this->num_attr = num_attr;
    this->n = n;
    this->num_threads = num_threads;
    this->path_eval = path_eval;
    this->comparison = comparison;
    this->code_length = find_code_length(n, hamming_weight);
    this->write_to_file = write_to_file;
    this->num_leaves = 1;
    this->log_poly_mod_degree = 14;
    if (comparison == RANGE_COVER && 2<=hamming_weight && hamming_weight<=4) this->log_poly_mod_degree=13;
    if (comparison == RANGE_COVER && hamming_weight==1) this->log_poly_mod_degree=12;
    this->poly_mod_degree = 1 << this->log_poly_mod_degree;
    this->row_count = 1 << (this->log_poly_mod_degree-1);
    this->slot_count = this->poly_mod_degree;
    this->path_eval=SUM;
    this->num_slots_per_element=2*this->n+1; 
    this->max_comps_per_ct = this->row_count / this->num_slots_per_element;
    this->reps_in_ct_per_attr = this->max_comps_per_ct / this->num_attr;
}

void QueryParameters::set_tree_params(DecisionTreeNode* root){
    std::queue<DecisionTreeNode*> traversal_queue;
    traversal_queue.push(root);

    std::map<uint64_t, int> attribute_counts;
    while (!traversal_queue.empty()) {
        DecisionTreeNode* current_element = traversal_queue.front();
        traversal_queue.pop();
        
        if (!current_element->is_leaf()){
            this->num_interal_nodes += 1;
            uint64_t au = current_element->attribute_used;
            if (attribute_counts.count(au) == 0) {
                attribute_counts[au] = 1;
            } else {
                attribute_counts[au] = attribute_counts[au] + 1;
            }
            traversal_queue.push(current_element->left_child);
            traversal_queue.push(current_element->right_child);
        }
    }
    int max_used = 0;
    for (std::map<uint64_t, int>::iterator it = attribute_counts.begin(); it != attribute_counts.end(); ++it) {
        max_used = max(max_used, it->second);
    }
    this->max_repetitions=max_used;
}

void permute_ret_vec(std::vector<std::pair<Ciphertext, Ciphertext>>& ret_vec)
{
    auto rd = std::random_device {};
    auto rng = std::default_random_engine { rd() };
    std::shuffle(std::begin(ret_vec), std::end(ret_vec), rng);
}

void QueryParameters::add_parameters_to_metrics()
{
    this->metrics_["bitlength"] = this->n;
    this->metrics_["num_attr"] = this->num_attr;
    this->metrics_["hamming_weight"] = this->k;
    this->metrics_["code_length"] = this->code_length;
    this->metrics_["num_threads"] = num_threads;
    this->metrics_["log_poly_mod_degree"] = this->log_poly_mod_degree;
    this->metrics_["comparison"] = this->comparison;
    this->metrics_["max_repetitions"] = this->max_repetitions;
    this->metrics_["num_internal_nodes"] = this->num_interal_nodes;
}

std::vector<uint64_t> get_leaf_classification_values(DecisionTreeNode* root)
{
    std::queue<DecisionTreeNode*> traversal_queue;
    std::vector<uint64_t> ret_vec;
    traversal_queue.push(root);
    while (!traversal_queue.empty()) {
        DecisionTreeNode* current_element = traversal_queue.front();
        traversal_queue.pop();
        if (!current_element->is_leaf()) {
            traversal_queue.push(current_element->left_child);
            traversal_queue.push(current_element->right_child);
        } else {
            ret_vec.push_back(current_element->threshold_value);
        }
    }
    return ret_vec;
}
