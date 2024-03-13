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
    //server_input   client_input
    //a              b
    //a       >      b

//g++ -o cdcmp -O3 cdcmp.cpp ../src/cmp.cpp -I ../include -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(int argc, char* argv[]){
    int n;// = 32; // n = 2 4 8 16 32 64 128 256 512
    int opt;
    while ((opt = getopt(argc, argv, "fn:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
    }
    }
    stringstream client_send;
    stringstream server_send;
    long client_send_commun=0;
    long server_send_commun=0;
    EncryptionParameters parms = cdcmp_init(n);
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
    cout << "Plaintext matrix row size: " << row_count << endl;
    uint64_t num_slots_per_element = n;
    uint64_t num_cmps_per_row = row_count / n; 
    uint64_t num_cmps = 2 * num_cmps_per_row;
    //cout<<"num_cmps "<<num_cmps<<endl;
    
    vector<uint64_t> encrypted_op; 
    vector<uint64_t> plain_op;
    vector<uint64_t> encrypted_op_encode; 
    vector<uint64_t> plain_op_encode;
    Ciphertext client_input;
    Plaintext server_input;

    srand(time(NULL));
    if(n < 64){
        for(int i = 0; i<num_cmps ;i++){
            encrypted_op.push_back( (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n));
        }
        for(int i = 0; i<num_cmps ;i++){
            plain_op.push_back( (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n));
        }
        encrypted_op_encode = cdcmp_encode_b(encrypted_op,n,slot_count,row_count,num_cmps_per_row);
        Plaintext plaintext; 
        batch_encoder->encode(encrypted_op_encode, plaintext);
        encryptor->encrypt(plaintext, client_input);

        plain_op_encode = cdcmp_encode_a(plain_op,num_slots_per_element,slot_count,row_count,num_cmps_per_row);
        batch_encoder->encode(plain_op_encode, server_input);
    }else{
        for(int i=0;i<slot_count;i++){
            encrypted_op_encode.push_back(1 - rand()%2);
        }
        for(int i=0;i<slot_count;i++){
            plain_op_encode.push_back(rand()%2);
        }
        Plaintext plaintext; 
        batch_encoder->encode(encrypted_op_encode, plaintext);
        encryptor->encrypt(plaintext, client_input);
        batch_encoder->encode(plain_op_encode, server_input);
    }


    client_send_commun+=client_input.save(client_send);

    clock_t start=clock();
    Ciphertext result = cdcmp(evaluator,gal_keys_server,rlk_server,num_slots_per_element,server_input,client_input);
    auto finish = clock()-start;
    
    server_send_commun+=result.save(server_send);


    
    Plaintext one_zero = init_one_zero_salt(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    //result = clear_cipher_result(evaluator, rlk_server, result, one_zero);
    vector<uint64_t> ans = cdcmp_decode_a_gt_b_dec(result,decryptor,batch_encoder,num_cmps,num_slots_per_element,num_cmps_per_row,row_count);

    bool correct = true;
    for(int i = 0; i < num_cmps ; i++){
        uint64_t actural_result = 0;
        //plain_op > encrypted_op[*][j]
        for(int k = i*n + n-1;k>=i*n;k--){
            if(plain_op_encode[k] > 1 - encrypted_op_encode[k]){
                actural_result = 1;
                break;
            }else if(plain_op_encode[k] == 1 - encrypted_op_encode[k]){
                continue;
            }else{
                break;
            }
        }
        
        if( !( ans[i] == actural_result ) ){
            cout<<"In "<< i <<" row is  error "<< endl;
            cout<<ans[i]  <<"  "<<actural_result << endl;
            correct = false;
            exit(0);
        }            
    }

long comm = client_send_commun + server_send_commun;
    //输出结果
cout<<    " decrypt result res[0] == b > a            : "<< correct 
    << ",\n bit precision                             : "<< n 
    << ",\n compare number                            : "<< num_cmps 
    << ",\n overall time cost                         : "<< finish/1000     
    << " ms\n overall comm. cost                        : "<< comm/1000 
    << " KB\n amortized time cost                       : "<< (float)finish/1000/num_cmps 
    << " ms\n amortized comm. cost                      : "<< (float)comm/1000 /num_cmps <<" KB"
    << endl;
    return 0;
}
