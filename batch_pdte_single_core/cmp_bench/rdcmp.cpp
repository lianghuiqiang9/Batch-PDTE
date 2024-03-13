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

//g++ -o rdcmp -O3 rdcmp.cpp ../src/cmp.cpp -I ../include -I /usr/local/include/SEAL-4.1 -lseal-4.1

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
    EncryptionParameters parms = rdcmp_init(n);
    //BCMP初始化公私钥
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

    //Encode plain_op and encrypted_op
    uint64_t num_cmps = slot_count - 1;//Reserve a padding bit to prevent the plaintext from being 0
    //cout<<"num_cmps "<<num_cmps<<endl;
    
    vector<uint64_t> encrypted_op; 
    vector<uint64_t> plain_op;
    vector<vector<uint64_t>> encrypted_op_encode; 
    vector<vector<uint64_t>> plain_op_encode; 
    vector<Ciphertext> client_input(n);
    vector<Plaintext> server_input(n);

    srand(time(NULL));
    if(n <= 64){
        for(int i = 0; i<num_cmps ;i++){
            encrypted_op.push_back( (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n));
        }
        for(int i = 0; i<num_cmps ;i++){
            plain_op.push_back( (uint64_t)(rand() + (1<<31) * rand()) % ((uint64_t)1 << n));
        }
        encrypted_op_encode = rdcmp_encode_b(encrypted_op,n,slot_count,row_count);
        Plaintext plaintext; 
        for(int i = 0 ; i < n; i++){
            batch_encoder->encode(encrypted_op_encode[i], plaintext);
            encryptor->encrypt(plaintext, client_input[i]);
        }
        //
        plain_op_encode = rdcmp_encode_a(plain_op,n,slot_count,row_count);
        for(int i = 0 ; i < n; i++){
            batch_encoder->encode(plain_op_encode[i], server_input[i]);
        }
    }else{
        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps,0);
            for(int j = 0; j<num_cmps; j++){
                temp[j] = 1 - rand()%2; // 1-b
            }
            encrypted_op_encode.push_back(temp);
        }

        Plaintext plaintext; 
        for(int i = 0 ; i < n; i++){
            batch_encoder->encode(encrypted_op_encode[i], plaintext);
            encryptor->encrypt(plaintext, client_input[i]);
        }

        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps,0);
            for(int j = 0; j<num_cmps; j++){
                temp[j] = rand()%2; //a
            }
            plain_op_encode.push_back(temp);
        }
        for(int i = 0 ; i < n; i++){
            batch_encoder->encode(plain_op_encode[i], server_input[i]);
        }
    }

    for(auto e:client_input){

        client_send_commun+=e.save(client_send);

    }

    clock_t start = clock();
    Ciphertext comparisonResult = rdcmp(evaluator,rlk_server,n,server_input,client_input);
    auto finish = clock()-start;

    server_send_commun+=comparisonResult.save(server_send);

    Plaintext pt;
    decryptor->decrypt(comparisonResult, pt);
    vector<uint64_t> ans;
    batch_encoder->decode(pt, ans);

    bool correct = true;
    for(int i = 0;i < num_cmps;i++){
        uint64_t actural_result = 0;
        //plain_op > encrypted_op[*][j]
        for(int k = n-1;k>=0;k--){
            if(plain_op_encode[k][i] > (1 - encrypted_op_encode[k][i])){
                actural_result = 1;
                break;
            }else if(plain_op_encode[k][i] == (1 - encrypted_op_encode[k][i])){
                continue;
            }else{
                break;
            }
        }
        if (!(ans[i] == actural_result)){
            cout<<"In "<< i <<" row is  error "<< endl;
            cout<<ans[i]  <<"  "<<actural_result << endl;
            correct = false;
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
