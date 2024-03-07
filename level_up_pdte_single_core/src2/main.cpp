#include "utils.h"
#include "client.h"
#include "server.h"

#include <fstream>
#include <iostream>
#include <unistd.h>
//#include <omp.h>

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


//./main -v -m ../../data/heart_11bits/model.json -a ../../data/heart_11bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 11


// example: ./main -v -m tree.in -a attr_vec.in -n 14
int main(int argc, char *argv[]) {

    //omp_set_num_threads(2);

    string model_fname;
    vector<uint64_t> attr_vec;
    Config config;
    string err_msg;

    int opt;
    while((opt = getopt(argc, argv, "dvs:m:a:n:")) != -1) {
        switch(opt) {
            case 'd': {
                config.debug_mode = true;
                break;
            }
            case 'v': {
                config.verbose = true;
                break;
            }
            case 's': {
                config.write_to_file = string(optarg);
                break;
            }
            case 'm': {
                model_fname = string(optarg);
                break;
            }
            case 'a': {
                //read_attr_vec(optarg, attr_vec);
                vector<vector<uint64_t>> client_data = read_csv_to_vector(string(optarg), 3);
                attr_vec = client_data[0];
                for(auto e: attr_vec){
                    cout<<e<<" ";
                }cout<<endl;
                break;
            }
            case 'n': {
                config.bitlength = stoi(string(optarg));
                break;
            }
        }
    }

    if(model_fname.empty()) {
        err_msg = "Empty classification model.";
    } else if(attr_vec.empty()) {
        err_msg = "Empty attribute vector.";
    } else if(config.bitlength < 1 || config.bitlength > 36) {
        err_msg = "Invalid bitlength, or you didn't specify one.";
    }

    if(!err_msg.empty()) {
        cerr << err_msg << endl;
        return 0;
    }

    auto server = Server();
    //server.fetch_model(model_fname);
    server.fetch_model_for_benchmark(model_fname);

    Client client(&server);
    client.run_protocol(attr_vec, config);
}
