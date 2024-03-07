#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include "node.h"

using namespace std;

//g++ -o build_tree -O3 build_tree.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp -I./include -I /usr/local/include/SEAL-4.1 -lseal-4.1
// if you use terminal to run, make sure the address and outputfile in a right position.
// like string address = "data/breast_11bits";string outputfile = "data/breast_31bits";


int main(){

    string address = "../data/breast_11bits";
    string address_data= address + "/x_test.csv";
    string outputfile = "../data/breast_31bits";
    int bit_length = 31;
    cout<<"1 << bit_length : "<<(1<<bit_length)<<endl;
    
    vector<vector<uint64_t>> client_data = read_csv_to_vector(address_data,100);
    for(int i=0;i<client_data.size();i++){
        for(int j=0;j<client_data[i].size();j++){
            client_data[i][j] = rand()%(1<<bit_length);
        }
    }
    for(int i=0;i<client_data.size();i++){
        for(int j=0;j<client_data[i].size() - 1;j++){
            cout<<client_data[i][j]<<",";
        }
        cout<<client_data[i].back()<<endl;
    }cout<<endl;

    cout<<"load tree"<<endl;// 
    string address_tree = address + "/model.json";//"../data/heart_11bits/model.json";
    Node root = Node(address_tree);
    cout<<"print tree"<<endl;
    print_tree(root);
    cout<<"root.leaf_num()  = "<<root.leaf_num()<<endl;

    json tree_json = root.to_json_with_random_value(bit_length);
    std::ofstream file(outputfile + "/model.json");
    file << tree_json << std::endl;  
    std::cout << "Tree has been saved to tree.json." << std::endl;

    Node root2 = Node(outputfile + "/model.json");
    cout<<"print tree"<<endl;
    print_tree(root2);
    cout<<"root.leaf_num()  = "<<root2.leaf_num()<<endl;


}