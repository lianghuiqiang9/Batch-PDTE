#include<seal/seal.h>
#include <cassert>
#include <iomanip>
#include <vector>
using namespace std;
using namespace seal;

// l=8  depth = 8  max_depth = 11   degree = 14 prime_bitlength = 17 coeff_modulus = 60       average time 39293\mus
// l=4  depth = 4  max_depth = 7    degree = 14 prime_bitlength = 17 coeff_modulus = 48       average time **\mus
// l=2  depth = 2  max_depth = 4    degree = 13 prime_bitlength = 20 coeff_modulus = default  average time **\mus

//g++ -o time_test -O3 time_test.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(){

    int m = 3;
    //int l = 4; uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=17; 
    int l = 4; uint64_t log_poly_mod_degree=14; uint64_t prime_bitlength=20;//适用于12比特的数据进行比较
    //int l=2; uint64_t log_poly_mod_degree=13; uint64_t prime_bitlength=20;
    int n = l * m;

    uint64_t poly_modulus_degree = 1 << log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    //parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    //parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));

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
    cout << "Plaintext matrix row size             : " << row_count << endl;

    uint64_t m_degree = 1<<m ;
    uint64_t num_slots_per_element = m_degree + 1;
    cout << "Plaintext matrix num_slots_per_element: " << num_slots_per_element << endl;
    uint64_t num_cmps_per_row = row_count/num_slots_per_element;
    uint64_t num_cmps = 2 * num_cmps_per_row;


    srand(time(NULL));
    vector<uint64_t> encrypted_op(slot_count, 0); 
    for(int i = 0; i<num_cmps ;i++){
        encrypted_op[i]= rand() % ((uint64_t)1 << n);
    }

    vector<uint64_t> plain_op(slot_count, 0);
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

    batch_encoder->encode(plain_op, pt);
    Plaintext pt_k; 
    batch_encoder->encode(k, pt_k);

    Ciphertext ct_temp;
    start = clock();
    evaluator->rotate_rows(ct, 1, *gal_keys_server, ct_temp);
    finish = clock() - start ;
    cout<<"rotate_rows                           : "<< finish <<" \\mus"<<endl;

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
    evaluator->multiply(ct,ct,ct_temp);
    finish = clock() - start ;
    cout<<"multiply                              : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->multiply_inplace(ct,ct);
    finish = clock() - start ;
    cout<<"multiply_inplace                      : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_inplace(ct,ct);
    finish = clock() - start ;
    cout<<"add_inplace                           : "<< finish <<" \\mus"<<endl;

    start = clock();
    evaluator->add_plain_inplace(ct,pt);
    finish = clock() - start ;
    cout<<"add_plain_inplace                     : "<< finish <<" \\mus"<<endl;

    return 0;
}

/*
Plaintext matrix row size             : 8192
Plaintext matrix num_slots_per_element: 9
encode                                : 191 \mus
encrypt                               : 7174 \mus
rotate_rows                           : 9913 \mus
multiply_plain                        : 5293 \mus
multiply_plain_inplace                : 4795 \mus
multiply_plain with one               : 243 \mus
multiply_plain_inplace with one       : 188 \mus
multiply_plain                        : 4765 \mus
add_inplace                           : 121 \mus
add_plain_inplace                     : 263 \mus
*/
/*

*/