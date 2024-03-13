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

void pdte_tecmp_rec(vector<Ciphertext>& out,Node& node, Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys *rlk_server, vector<vector<Ciphertext>> client_input, Plaintext one,int l,int m,uint64_t m_degree, BatchEncoder *batch_encoder,int slot_count, int plain_modulus, seal::Ciphertext one_zero_init_cipher){
    if (node.is_leaf()){
        out.push_back(node.value);
    }else{
        //pearent     0           *  parent       1
        //left      0   1  right  *  left       1   0     right
        //note that the index of client_data in the tree starts from 0
        //cmp
        node.right->value = tecmp(evaluator, gal_keys_server,rlk_server, node.threshold_bitv, client_input[ node.feature_index ],l, m, m_degree, one_zero_init_cipher);
        evaluator->negate(node.right->value, node.left->value);
        evaluator->add_plain_inplace(node.left->value, one);
        //travel
        evaluator->add_inplace(node.left->value, node.value);
        evaluator->add_inplace(node.right->value, node.value);
        pdte_tecmp_rec(out, *(node.left), evaluator,gal_keys_server, rlk_server, client_input,one,l,m,m_degree,  batch_encoder,slot_count, plain_modulus,one_zero_init_cipher);
        pdte_tecmp_rec(out, *(node.right), evaluator,gal_keys_server, rlk_server, client_input,one,l,m,m_degree,  batch_encoder,slot_count, plain_modulus,one_zero_init_cipher);
    }
}

//g++ -o tecmp_pdte -O3 tecmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp -I./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(int argc, char* argv[]){
    if (argc < 3) {
        cout << "Need at least a model and a vector" << endl;
        return -1;
    }
    string address_data;
    string address_tree;
    int l,m;
    int clr;
    int data_m;
    int opt;
    while ((opt = getopt(argc, argv, "ft:v:r:l:m:c:")) != -1) {
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
        case 'l':
            l = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
            break;
        case 'c':
            clr = atoi(optarg);
            break;
    }
    }
    int n =l*m;

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
    cout<<"load the client_data,                 run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();
    cout<<"Init fhe ... " <<endl;
    

    EncryptionParameters parms;
    if(clr==1){
        parms = tecmp_init(n,2048,m);
    }else{
        parms = tecmp_init(n,l,m);
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
    uint64_t slot_count = batch_encoder->slot_count(); // 8192 16384
    uint64_t row_count = slot_count / 2;               // 4096 8192
    uint64_t m_degree = 1 << m ;
    uint64_t num_slots_per_element =  m_degree; 
    uint64_t num_cmps_per_row = row_count/num_slots_per_element;
    uint64_t num_cmps = 2 * num_cmps_per_row;

    cout << "Init fhe finish ... " <<endl;
    cout << "Plaintext matrix num_cmps:              " << num_cmps << endl;

    //data_m < num_cmps 
    if(num_cmps < data_m){
        cout<<"data_m : "<<data_m<<" >= num_cmps : "<<num_cmps<<endl;
        cout<<"data_m is too large, please divide different page until the size is small than num_cmps"<<endl;
        exit(0);
    }
    cout<<"Init the BFV batch scheme,            run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();

    vector<vector<uint64_t>> index_feat = Transpose(client_data); 
    vector<vector<Ciphertext>> client_input;
    for(int i = 0; i < index_feat.size(); i++){
        vector<vector<uint64_t>> encrypted_op_encode = tecmp_encode_b(index_feat[i],l,m,m_degree,num_cmps);
        vector<Ciphertext> temp = tecmp_thermometer_encode_b_enc(encrypted_op_encode, batch_encoder,encryptor,l,m,slot_count,row_count,m_degree,num_cmps_per_row,num_slots_per_element);
        client_input.push_back(temp);
    }

    Plaintext one_zero_init = init_one_zero_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    Ciphertext one_zero_init_cipher;
    encryptor->encrypt(one_zero_init,one_zero_init_cipher);

    cout<<"encrypt the client data ,             run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();

    cout<<"******************************* step 1: client end *******************************"<<endl;
    //evaluater
    //client_input
    for(auto e:client_input){
        for(auto f:e){
            client_send_commun+=f.save(client_send);
        }
    }

    cout<<"******************************* step 2: server begin *******************************"<<endl;

    cout<<"load tree"<<endl;
    Node root = Node(address_tree);
    cout<<"root.leaf_num()  = "<<root.leaf_num()<<endl;

    Plaintext one = init_one(batch_encoder, slot_count);
    root.value = init_cipher_zero(batch_encoder, encryptor, slot_count);
    root.tecmp_pdte_init(l, m);

    cout<<"Init the tree,                          run time is "<<(clock()-start) /1000<<" ms"<<endl;

    vector<Ciphertext> out1;
    start = clock();
    pdte_tecmp_rec( out1, root, evaluator, gal_keys_server, rlk_server, client_input, one, l, m,m_degree, batch_encoder, slot_count, plain_modulus, one_zero_init_cipher);

    vector<uint64_t> leaf_vec;
    leaf_extract_rec(leaf_vec, root);
    int leaf_num = leaf_vec.size();


    vector<Plaintext> leaf_vec_plain;//leaf_vec_plain
    for(int i = 0; i<leaf_num; i++){
        Plaintext pt = init_b_zero_salt(batch_encoder,leaf_vec[i],slot_count,data_m,num_cmps_per_row,num_slots_per_element,row_count); 
        leaf_vec_plain.push_back(pt);
    }

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
        res0 = tecmp_decode_a_gt_b_dec(out[0][j],decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);
        res1 = tecmp_decode_a_gt_b_dec(out[1][j],decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);
        ans0.push_back(res0);
        ans1.push_back(res1);
    }

    cout<<"decrypt the result ,                 run time is "<<(clock()-start) <<"\\mus"<<endl;start = clock();

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

    if(expect_result.size()==0){
        cout<<"data_m is too large, please choose a small one"<<endl;
    }

    for(int j = 0; j < data_m ; j++){
        //cout<<"j = "<<j<<" ";
        uint64_t actural_result = root.eval(client_data[j]);
        //cout<< expect_result[j] <<" "<<actural_result<< endl;
        if( !( expect_result[j] == actural_result ) ){
            cout<<"In "<< j <<" col is  error "<< endl;
            cout<<expect_result[j]  <<"  "<<actural_result << endl;
            exit(0);
        }            
    }

    long comm = client_send_commun + server_send_commun;
    cout<<"compare with the real result , run time is "<<(float)(clock()-start)/1000 <<" ms"<<endl;start = clock();
    cout<<"*********************************************************************************"<<endl;
    cout<<"address_tree : "<<address_tree<<endl;
    cout<<"l            : "<<l<<endl;
    cout<<"m            : "<<m<<endl;
    cout<<"n            : "<<n <<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall run time is "<< finish/1000 <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall commun.  is "<< comm/1000 <<" KB"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average run time is "<< (float)finish/1000/data_m <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average commun.  is "<< comm/1000/data_m <<" KB"<<endl;
    cout<<"******************************* step 3: client end *******************************"<<endl;
    cout<<"all done, overall run time is "<< (clock() - global_start)/1000 <<" ms"<<endl;

    return 0;
}

