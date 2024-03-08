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
// degree = 13 prime_bitlength = 17 coeff_modulus = default  max_depth = 5    multiply_inplace average time 11909\mus
// degree = 13 prime_bitlength = 20 coeff_modulus = default  max_depth = 4    multiply_inplace average time 12180\mus

//g++ -o depth_test -O3 depth_test.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(){
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17; 
    //uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=13; uint64_t prime_bitlength=17;
    //uint64_t log_poly_mod_degree=12; uint64_t prime_bitlength=16;
    uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17;

    uint64_t poly_modulus_degree = 1 << log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    //parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    
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
    int l=2;
    int m=4;
    int n = l * m;
    uint64_t m_degree = 1<<m ;
    uint64_t num_slots_per_element = m_degree;
    //out << "Plaintext matrix num_slots_per_element: " << num_slots_per_element << endl;
    uint64_t num_cmps_per_row = row_count/num_slots_per_element;
    uint64_t num_cmps = 2 * num_cmps_per_row;



    vector<uint64_t> x(slot_count, 1);
    x[1] = 3;
    vector<uint64_t> y=x;

    Plaintext pt; 
    Ciphertext ct,ct_temp;

    auto start = clock();
    batch_encoder->encode(x, pt);

    encryptor->encrypt(pt, ct);


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
            cout<<"depth                                 : "<<depth<<endl;
            break;
        }

    }

   {
    srand(time(NULL));
    vector<uint64_t> encrypted_op(slot_count, 0); 
    for(int i = 0; i<num_cmps ;i++){
        encrypted_op[i]= rand() % ((uint64_t)1 << n);
    }

    vector<uint64_t> plain_op(slot_count, 0);
    for(int i = 0; i<num_cmps ;i++){
        plain_op[i]= rand() % ((uint64_t)1 << n);
    }

    vector<uint64_t> slat(slot_count, 0);
    for(int i = 0; i<num_cmps ;i++){
        plain_op[i]= rand() % ((uint64_t)1 << n);
    }

    vector<uint64_t> k(slot_count, 1);

    Plaintext pt; 
    Ciphertext ct;

    auto start = clock();
    batch_encoder->encode(encrypted_op, pt);
    auto finish = clock() - start ;
    cout<<"encode                                : "<< finish <<" \\mus"<<endl;
    
    start = clock();
    encryptor->encrypt(pt, ct);
    finish = clock() - start ;
    cout<<"encrypt                               : "<< finish <<" \\mus"<<endl;

    start = clock();
    decryptor->decrypt(ct,ans);
    finish = clock() - start ;
    cout<<"decrypt                               : "<< finish <<" \\mus"<<endl;

    start = clock();
    batch_encoder->decode(ans, res);
    finish = clock() - start ;
    cout<<"decode                                : "<< finish <<" \\mus"<<endl;

    batch_encoder->encode(plain_op, pt);
    Plaintext pt_k; 
    batch_encoder->encode(k, pt_k);

    Ciphertext ct_temp;
    start = clock();
    evaluator->rotate_rows(ct, 1, *gal_keys_server, ct_temp);
    finish = clock() - start ;
    cout<<"rotate_rows                           : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply(ct,ct,ct_temp);
    finish = clock() - start ;
    cout<<"multiply                              : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_inplace(ct,ct);
    finish = clock() - start ;
    cout<<"multiply_inplace                      : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_plain(ct,pt,ct_temp);
    finish = clock() - start ;
    cout<<"multiply_plain                        : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_plain_inplace(ct,pt);
    finish = clock() - start ;
    cout<<"multiply_plain_inplace                : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_plain(ct,pt_k,ct_temp);
    finish = clock() - start ;
    cout<<"multiply_plain with one               : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_plain_inplace(ct,pt_k);
    finish = clock() - start ;
    cout<<"multiply_plain_inplace with one       : "<< finish <<" \\mus"<<endl;



    start = clock();
    evaluator->add(ct,ct,ct_temp);
    finish = clock() - start ;
    cout<<"add                                   : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_inplace(ct,ct);
    finish = clock() - start ;
    cout<<"add_inplace                           : "<< finish <<" \\mus"<<endl;



    start = clock();
    evaluator->add_plain(ct,pt,ct_temp);
    finish = clock() - start ;
    cout<<"add_plain                             : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_plain_inplace(ct,pt);
    finish = clock() - start ;
    cout<<"add_plain_inplace                     : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_plain(ct,pt_k,ct_temp);
    finish = clock() - start ;
    cout<<"add_inplace with one                  : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_plain_inplace(ct,pt_k);
    finish = clock() - start ;
    cout<<"add_plain_inplace with one            : "<< finish <<" \\mus"<<endl;


    Ciphertext ct2;

    batch_encoder->encode(slat, pt);
    encryptor->encrypt(pt, ct2);

    start = clock();
    evaluator->sub(ct2,ct_temp,ct_temp);
    finish = clock() - start ;
    cout<<"sub                                   : "<< finish <<" \\mus"<<endl;
    
    start = clock();
    evaluator->sub_inplace(ct,ct_temp);
    finish = clock() - start ;
    cout<<"sub_inplace                           : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->sub_plain(ct,pt,ct_temp);
    finish = clock() - start ;
    cout<<"sub_plain                             : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->sub_plain_inplace(ct,pt);
    finish = clock() - start ;
    cout<<"sub_plain_inplace                     : "<< finish <<" \\mus"<<endl;


   }
    stringstream client_send;
    long client_send_commun=0;
    client_send_commun+=ct.save(client_send);
    cout<<"comm. cost                            : "<< client_send_commun/1000 << " KB"<<endl;
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