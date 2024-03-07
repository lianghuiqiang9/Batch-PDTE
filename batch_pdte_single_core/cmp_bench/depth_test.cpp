#include<seal/seal.h>
#include <cassert>
#include <iomanip>
#include <vector>
#include<iostream>
#include <fstream>
using namespace std;
using namespace seal;

// degree = 14 prime_bitlength = 17 coeff_modulus = default  max_depth = 12   multiply_inplace average time 54180\mus
// degree = 14 prime_bitlength = 17 coeff_modulus = 60       max_depth = 11   multiply_inplace average time 38729\mus
// degree = 14 prime_bitlength = 17 coeff_modulus = 48       max_depth = 8    multiply_inplace average time 38344\mus
// degree = 13 prime_bitlength = 20 coeff_modulus = default  max_depth = 4    multiply_inplace average time 12180\mus

//g++ -o depth_test -O3 depth_test.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(){
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17; 
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=13; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=12; uint64_t prime_bitlength=16;
    uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=20;

    uint64_t poly_modulus_degree = 1 << log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    //parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    //parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    
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
    uint64_t slot_count = batch_encoder->slot_count();
    uint64_t plain_modulus = parms.plain_modulus().value();

    vector<uint64_t> x(slot_count, 1);
    x[1] = 3;
    vector<uint64_t> y=x;

    Plaintext pt; 
    Ciphertext ct,ct_temp;

    auto start = clock();
    batch_encoder->encode(x, pt);
    auto finish = clock() - start ;
    cout<<"encode                                : "<< finish <<" \\mus"<<endl;

    start = clock();
    encryptor->encrypt(pt, ct);
    finish = clock() - start ;
    cout<<"encrypt                               : "<< finish <<" \\mus"<<endl;


    encryptor->encrypt(pt, ct_temp);
    Plaintext ans; 
    vector<uint64_t> res;
    int depth = 0;
    start = clock();
    for(int i = 0;i<20;i++){
        evaluator->multiply_inplace(ct,ct_temp);
        evaluator->relinearize_inplace(ct, *rlk_server);

        for(int j=0;j<slot_count;j++){
            x[j] = x[j]*y[j] % plain_modulus;
        }
        decryptor->decrypt(ct,ans);
        batch_encoder->decode(ans, res);
        //cout<<res[0]<<" "<<res[1]<<endl<<x[0]<<" "<<x[1]<<endl;
        if (res[0]!=x[0]||res[1]!=x[1]){
            depth = i;
            cout<<"depth = "<<depth<<endl;
            break;
        }

    }
    finish = clock() - start ;
    cout<<"average multiply_inplace                          : "<< finish/(depth+1) <<" \\mus"<<endl;




    //cout<<" log_poly_mod_degree     = "<<log_poly_mod_degree<<endl;
    //cout<<" log_plain_modlus        = "<<log(parms.plain_modulus().value())/log(2)<<endl;
    //cout<<" log_cipher_coeff_modlus = "<<log(parms.coeff_modulus()[0].value())/log(2)<<endl;
    //cout<<" log_cipher_coeff_modlus = "<<log(parms.coeff_modulus()[1].value())/log(2)<<endl;
    //cout<<" log_cipher_coeff_modlus = "<<log(parms.coeff_modulus()[2].value())/log(2)<<endl;

    //std::ofstream output_file("bfv_ciphertext", ios::binary);
    //cout<<ct.save(output_file)<<endl;


    return 0;
}
/*
88K     ./bfv_ciphertext_degree_12_bit_16_mod_36_depth_1.txt
424K    ./bfv_ciphertext_degree_13_bit_17_mod_44_depth_5.txt
1.8M    ./bfv_ciphertext_degree_14_bit_17_mod_48_depth_12.txt
1.3M    ./bfv_ciphertext_degree_14_bit_17_mod_48_depth_8.txt
1.6M    ./bfv_ciphertext_degree_14_bit_17_mod_60_depth_11.txt

show the storage
du ./* -s -h
*/