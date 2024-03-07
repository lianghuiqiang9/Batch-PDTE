#include "client.h"
#include "seal/seal.h"
#include "server.h"
#include "tree_utils.h"
#include "utils.h"
#include "parser.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <math.h>
#include <random>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace seal;
using namespace std;


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
    //for(int i=0;i<data_m;i++){
    while(i<data_m && std::getline(file, line)){
        //std::getline(file, line);
        std::vector<uint64_t> row;
        std::stringstream lineStream(line);
        std::string cell;

        // 按逗号分隔值并将它们转换为整数并存储到向量中
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

//./main  -t ../../experiments/datasets_quantized/breast/tree_breast_n_2.json -v ../../experiments/datasets_quantized/breast/input_breast_n_2.txt -s ../../experiments/results-v9/src1/breast/ -n 2 -w 0 -e 1


int main(int argc, char* argv[])
{

    if (argc < 3) {
        cout << "Need at least a model and a vector" << endl;
        return -1;
    }

    string model_filename;
    string attribute_vec_filename;

    int opt;
    int bitlength = 32;
    int hamming_weight = 8;
    bool flexible_mode = false;
    PathEvaluation path_eval;
    ComparisonType comparison=RANGE_COVER;
    string write_to_file = "results/"; // The directory to write to (include forward slash)
    while ((opt = getopt(argc, argv, "fs:t:v:w:n:pe:")) != -1) {
        switch (opt) {
        case 's':
            write_to_file = string(optarg);
            break;
        case 't':
            model_filename = string(optarg);
            break;
        case 'v':
            attribute_vec_filename = string(optarg);
            break;
        case 'p':
            path_eval=SUM;
            break;
        case 'w':
            hamming_weight = atoi(optarg);
            break;
        case 'n':
            bitlength = atoi(optarg);
            break;
        case 'e':
            switch (stoi(optarg)){
                case 0:
                    comparison=RANGE_COVER;
                    cout<<"comparison : RANGE_COVER"<<endl;
                    break;
                case 1:
                    comparison=FOLKLORE;
                    cout<<"comparison : FOLKLORE"<<endl;
                    break;
                default:
                    comparison=RANGE_COVER;
                    cout<<"comparison : RANGE_COVER"<<endl;
            }
            break;
        case ':':
            std::cout << "option needs a value: " << string(optarg) << endl;
            break;
        case '?':
            std::cout << "unknown option: " << optopt << "\n";
            break;
        }
    }
    cout<<"attribute_vec_filename : "<< attribute_vec_filename<<endl;
    vector<vector<uint64_t>> client_data = read_csv_to_vector(attribute_vec_filename, 3);
    vector<uint64_t> input = client_data[0];
    for(auto e: input){
        cout<<e<<" ";
    }cout<<endl;
    //vector<uint64_t> input = read_input_from_file(attribute_vec_filename);

    QueryParameters* query_parameters = new QueryParameters(bitlength, input.size(), hamming_weight, 2, path_eval, comparison, write_to_file);
    Client* client = new Client();
    client->server->initialize_model(model_filename, query_parameters);
    client->end_to_end_evaluation(query_parameters, input, true);
    delete query_parameters;

    cout<<"comparison : "<< comparison<<endl;
}
