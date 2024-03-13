#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include "node.h"
#include "utils.h"
#include "cmp.h"
#include "pdte.h"
#include<seal/seal.h>

using namespace std;
using namespace seal;

void pdte_cdcmp_rec(vector<Ciphertext>& out,Node& node, Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys *rlk_server, vector<Ciphertext> client_input, Plaintext one, BatchEncoder *batch_encoder, int num_cmps, int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row){
    if (node.is_leaf()){
        out.push_back(node.value);
    }else{
        //pearent     0           *  parent       1
        //left      0   1  right  *  left       1   0     right
        //note that the index of client_data in the tree starts from 0
        //cmp
        node.right->value = cdcmp(evaluator, gal_keys_server, rlk_server, num_slots_per_element, node.threshold_bitv_plain[0], client_input[ node.feature_index ]);
        evaluator->negate(node.right->value, node.left->value);
        evaluator->add_plain_inplace(node.left->value, one);
        //travel
        evaluator->add_inplace(node.left->value, node.value);
        evaluator->add_inplace(node.right->value, node.value);
        pdte_cdcmp_rec(out, *(node.left), evaluator,gal_keys_server, rlk_server, client_input,one, batch_encoder,num_cmps, num_slots_per_element, slot_count, row_count, num_cmps_per_row);
        pdte_cdcmp_rec(out, *(node.right), evaluator,gal_keys_server, rlk_server, client_input,one, batch_encoder,num_cmps, num_slots_per_element, slot_count, row_count, num_cmps_per_row);
    }
}


//g++ -o cdcmp_pdte -O3 cdcmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp -I./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(int argc, char* argv[]){


    string address_data;
    string address_tree;
    int n;
    int clr;
    int data_m;
    int opt;
    while ((opt = getopt(argc, argv, "ft:v:r:n:c:")) != -1) {
        switch (opt) {
        case 't':
            address_tree = string(optarg);
            break;
        case 'v':
            address_data = string(optarg);
            break;
        case 'r':
            data_m = atoi(optarg);
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 'c':
            clr = atoi(optarg);
            break;
    }
    }
    stringstream client_send;
    stringstream server_send;
    long client_send_commun=0;
    long server_send_commun=0;
    cout<<"******************************* step 1: client begin *******************************"<<endl;
    clock_t global_start = clock();
    clock_t start = clock();

    vector<vector<uint64_t>> client_data = read_csv_to_vector(address_data, data_m);
    data_m = client_data.size();
    int data_n = client_data[0].size();
    cout<<"data_m    = "<<data_m<<endl;
    cout<<"data_n    = "<<data_n<<endl;//print_data(client_data); 

    cout<<"load the client_data,                 run time is "<<(clock()-start)/1000 <<" ms"<<endl;start = clock();
    cout<<"Init fhe ... " <<endl;
    
    EncryptionParameters parms;
    if(clr==1){
        parms = cdcmp_init(1024);
    }else{
        parms = cdcmp_init(n);
    }
    
    SEALContext* context = new SEALContext(parms);
    KeyGenerator keygen(*context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    Encryptor *encryptor = new Encryptor(*context, pk);
    Decryptor *decryptor = new Decryptor(*context, sk);
    Evaluator *evaluator = new Evaluator(*context);
    BatchEncoder* batch_encoder = new BatchEncoder(*context);
    RelinKeys* rlk_server = new RelinKeys();
    keygen.create_relin_keys(*rlk_server);
    GaloisKeys* gal_keys_server = new GaloisKeys();
    keygen.create_galois_keys(*gal_keys_server);    
    uint64_t plain_modulus = parms.plain_modulus().value();

    //explain slot info 
    uint64_t slot_count = batch_encoder->slot_count();
    uint64_t row_count = slot_count / 2;
    uint64_t num_slots_per_element = n;
    uint64_t num_cmps_per_row = row_count / num_slots_per_element; 
    uint64_t num_cmps = 2 * num_cmps_per_row;

    cout << "Init fhe finish ... " <<endl;
    cout << "Plaintext matrix num_cmps:              " << num_cmps << endl;

    //data_m < num_cmps 
    if(num_cmps < data_m){
        cout<<"data_m : "<<data_m<<" >= num_cmps : "<<num_cmps<<endl;
        cout<<"data_m is too large, please divide different page until the size is small than num_cmps"<<endl;
        exit(0);
    }
    cout<<"Init the BFV batch scheme,            run time is "<<(clock()-start)/1000 <<" ms"<<endl;start = clock();

    vector<vector<uint64_t>> index_feat_temp = Transpose(client_data); 
    vector<vector<uint64_t>> index_feat = Matrix_padding(index_feat_temp,num_cmps);

    vector<Ciphertext> client_input;
    for(int i = 0; i < index_feat.size(); i++){
        Ciphertext temp;
        vector<uint64_t> encrypted_op_encode = cdcmp_encode_b(index_feat[i],num_slots_per_element,slot_count,row_count,num_cmps_per_row);
        Plaintext plaintext; 
        batch_encoder->encode(encrypted_op_encode, plaintext);
        encryptor->encrypt(plaintext, temp);
        client_input.push_back(temp);
    }
    cout<<"encrypt the client data ,             run time is "<<(clock()-start)/1000 <<" ms"<<endl;start = clock();

    cout<<"******************************* step 1: client end *******************************"<<endl;
    //evaluater
    //client_input
    for(auto e:client_input){
        client_send_commun+=e.save(client_send);
    }

    cout<<"******************************* step 2: server begin *******************************"<<endl;

    cout<<"load tree"<<endl;
    Node root = Node(address_tree);

    Plaintext one = init_one(batch_encoder, slot_count);
    root.value = init_cipher_zero(batch_encoder, encryptor, slot_count);
    root.cdcmp_pdte_init(batch_encoder,num_cmps,num_slots_per_element,slot_count,row_count,num_cmps_per_row);

    cout<<"Init the tree,                          run time is "<<(clock()-start)/1000 <<" ms"<<endl;

    vector<Ciphertext> out1;
    start = clock();
    pdte_cdcmp_rec(out1, root, evaluator, gal_keys_server, rlk_server, client_input, one, batch_encoder,num_cmps, num_slots_per_element, slot_count, row_count, num_cmps_per_row);

    vector<uint64_t> leaf_vec;
    leaf_extract_rec(leaf_vec, root);
    int leaf_num = leaf_vec.size();
    cout<<"leaf_num         = "<<leaf_num<<endl;

    vector<Plaintext> leaf_vec_plain;//leaf_vec_plain
    for(int i = 0; i<leaf_num; i++){
        Plaintext pt = init_b_zero_salt(batch_encoder,leaf_vec[i],slot_count,data_m,num_cmps_per_row,num_slots_per_element,row_count); 
        leaf_vec_plain.push_back(pt);
    }

    //初始化随机数
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(1, plain_modulus - 1);
    
    vector<vector<Plaintext>> salt;//salt
    for(int i = 0; i <  2 ; i++){
        vector<Plaintext> temp;
        for(int j = 0;j < leaf_num; j++){
            temp.push_back(init_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count,distrib,gen));
        }
        salt.push_back(temp);
    }

    vector<vector<Ciphertext>> out(2, vector<Ciphertext>(0) );
    out[0] = out1;
    out[1] = out1;
    
    for(int i = 0; i < leaf_num; i++){
        evaluator->multiply_plain_inplace(out[0][i],salt[0][i]);
        evaluator->multiply_plain_inplace(out[1][i],salt[1][i]);
        evaluator->add_plain_inplace(out[1][i],leaf_vec_plain[i]);
    }

    Plaintext one_zero = init_one_zero_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    for(int i = 0;i < out.size();i++){
        for(int j = 0;j<out[0].size();j++){
            out[i][j] = clear_cipher_result(evaluator, rlk_server, out[i][j], one_zero);
        }
    }


    if(clr==1){
        cout<<"clr process"<<endl;
        out = tecmp_cdcmp_pdte_clear_line_relation(batch_encoder,evaluator,out,leaf_num,data_m,slot_count,row_count,num_cmps_per_row,num_slots_per_element);
    }
        
    clock_t finish = clock()-start; start = clock();

    cout<<"******************************* step 2: server end *******************************"<<endl;
    //out
    for(auto e:out){
        for(auto f:e){
            server_send_commun+=f.save(server_send);
        }
    }

    cout<<"******************************* step 3: client start *******************************"<<endl;

    vector<vector<uint64_t>> ans0, ans1;
    for(int j=0;j<out[0].size();j++){
        vector<uint64_t> res0, res1;
        res0 = cdcmp_decode_a_gt_b_dec(out[0][j],decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);
        res1 = cdcmp_decode_a_gt_b_dec(out[1][j],decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);
        ans0.push_back(res0);
        ans1.push_back(res1);
    }

    cout<<"decrypt the result ,                 run time is "<<(float)(clock()-start)/1000 <<"ms"<<endl;start = clock();

    cout<<"result compare"<<endl;

    vector<uint64_t> expect_result;
    for(int j = 0; j < data_m ; j++){
       for(int i = 0; i < ans0.size(); i++){
            if(ans0[i][j]==0){
                expect_result.push_back(ans1[i][j]);
                break;      
            }
        }
    }

    for(int j = 0; j < data_m ; j++){
        uint64_t actural_result = root.eval(client_data[j]);
        //cout<< expect_result[j] <<" "<<actural_result<< endl;
        if( !( expect_result[j] == actural_result ) ){
            cout<<"In "<< j <<" col is  error "<< endl;
            cout<<expect_result[j]  <<"  "<<actural_result << endl;
            exit(0);
        }            
    }
    long comm = client_send_commun + server_send_commun;
    cout<<"compare with the real result , run time is "<<(clock()-start)/1000 <<"ms"<<endl;start = clock();
    cout<<"*********************************************************************************"<<endl;
    cout<<"address_tree : "<<address_tree<<endl;
    cout<<"n            : "<< n <<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall run time is "<< finish/1000 <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall commun.  is "<< comm/1000 <<" KB"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average run time is "<< finish/1000/data_m <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average commun.  is "<< comm/1000/data_m <<" KB"<<endl;
    cout<<"******************************* step 3: client end *******************************"<<endl;
    cout<<"all done, overall run time is "<< (clock() - global_start)/1000 <<" ms"<<endl;

    return 0;
}

