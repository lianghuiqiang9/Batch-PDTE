#include<seal/seal.h>
#include <cassert>
#include <iomanip>
#include <vector>
#include "cmp.h"
#include<iostream>
#include <fstream>
using namespace std;
using namespace seal;

    //random two number for compare
    //plain_op       encrypted_op
    //server_values  attribute_vector
    //server_input   client_input
    //a              E(b)
    //a       >      E(b)



//g++ -o tecmp_norm -O3 tecmp_norm.cpp ../src/cmp.cpp  -I../include -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(int argc, char* argv[]){

    int l,m; 
    int opt;
    while ((opt = getopt(argc, argv, "fl:m:")) != -1) {
        switch (opt) {
        case 'l':
            l = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
            break;
    }
    }

    stringstream client_send;
    stringstream server_send;
    long client_send_commun=0;
    long server_send_commun=0;

    int n = l * m;
    EncryptionParameters parms = tecmp_norm_init(n,l,m);
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
    uint64_t slot_count = batch_encoder->slot_count();
    uint64_t row_count = slot_count / 2;
    //cout << "Plaintext matrix row size: " << row_count << endl;
    uint64_t m_degree = 1<<m ;
    uint64_t num_slots_per_element = m_degree;
    //out << "Plaintext matrix num_slots_per_element: " << num_slots_per_element << endl;
    uint64_t num_cmps_per_row = row_count/num_slots_per_element;
    uint64_t num_cmps = 2 * num_cmps_per_row;

    vector<vector<uint64_t>> encrypted_op_encode; 
    vector<uint64_t> plain_op_encode; 
    vector<Ciphertext> client_input;
    vector<uint64_t> server_input;
            
    //Encode plain_op and encrypted_op
    if(n<=64){
        srand(time(NULL));
        vector<uint64_t> encrypted_op(num_cmps, 0); //num_cmps_per_row number encrypted_op
        for(int i = 0; i<num_cmps ;i++){
            encrypted_op[i]= (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n);
        }
        cout<<" E(b)[0] = encrypted_op[0] = "<<encrypted_op[0]<<endl;
        uint64_t plain_op = (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n);
        cout<<" a       = plain_op        = "<<plain_op<<endl;
        encrypted_op_encode = tecmp_encode_b(encrypted_op,l,m,m_degree,num_cmps);
        client_input = tecmp_thermometer_encode_b_enc(encrypted_op_encode, batch_encoder,encryptor,l,m,slot_count,row_count,m_degree,num_cmps_per_row,num_slots_per_element);
        plain_op_encode = tecmp_encode_a(plain_op, l, m, m_degree);
        server_input = plain_op_encode;
    }else{
        for(int i = 0; i < l ;i++){
            vector<uint64_t> encrypted_op_temp(num_cmps, 0);
            for(int j = 0;j < num_cmps;j++){
                encrypted_op_temp[j]= rand() % (1 << m);  //rand 0  2^31-1
            }
            encrypted_op_encode.push_back(encrypted_op_temp);
        }
        cout<<" E(b)[0] = encrypted_op_encode[0] = ";
        for(int i = 0; i<l ;i++){cout<<encrypted_op_encode[i][0]<<" ";}cout<<endl;
        for(int i = 0; i < l ;i++){ plain_op_encode.push_back(rand() % (1 << m)); }
        cout<<" a       = plain_op_encode        = ";
        for(int i = 0; i < l ;i++){cout<<plain_op_encode[i]<<" ";}cout<<endl;
        client_input = tecmp_thermometer_encode_b_enc(encrypted_op_encode, batch_encoder,encryptor,l,m,slot_count,row_count,m_degree,num_cmps_per_row,num_slots_per_element);
        server_input = plain_op_encode;
    }

    for(auto e:client_input){
        client_send_commun+=e.save(client_send);

    }

    Plaintext one_zero_init = init_one_zero_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    Ciphertext one_zero_init_cipher;
    encryptor->encrypt(one_zero_init,one_zero_init_cipher);

    clock_t start=clock();
    Ciphertext result = tecmp_norm(evaluator, gal_keys_server, rlk_server, server_input, client_input, l, m,m_degree, one_zero_init_cipher);
    auto finish = clock()-start;

    Plaintext one_zero = init_one_zero_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    result = clear_cipher_result(evaluator, rlk_server, result, one_zero);

    server_send_commun+=result.save(server_send);

    vector<uint64_t> ans = tecmp_decode_a_gt_b_dec(result,decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);
    
    bool correct = true;
    for(int i = 0; i < num_cmps ; i++){
        uint64_t actural_result = 0;
        //plain_op > encrypted_op[*][j]
        for(int k = l-1;k>=0;k--){
            if(plain_op_encode[k]>encrypted_op_encode[k][i]){
                actural_result = 1;
                break;
            }else if(plain_op_encode[k]==encrypted_op_encode[k][i]){
                continue;
            }else{
                break;
            }
        }

        //cout<<"i : "<<i<<" : "<<ans[i]  <<"  "<<actural_result << endl;
        if( !( ans[i] == actural_result ) ){
            cout<<"In "<< i <<" row is  error "<< endl;
            cout<<ans[i]  <<"  "<<actural_result << endl;
            correct = false;
            //    //exit(0);
        }            
    }
    long comm = client_send_commun + server_send_commun;

cout<<    " decrypt result res[0] == b > a            : "<< correct 
    << ",\n l                                         : "<< l 
    << ",\n m                                         : "<< m
    << ",\n bit precision                             : "<< n 
    << ",\n compare number                            : "<< num_cmps 
    << ",\n overall time cost                         : "<< finish/1000     
    << " ms\n overall comm. cost                        : "<< comm/1000 
    << " KB\n amortized time cost                       : "<< (float)finish/1000/num_cmps 
    << " ms\n amortized comm. cost                      : "<< (float)comm/1000 /num_cmps <<" KB"
    << endl;


    return 0;
}
