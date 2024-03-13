#ifndef __CMP__
#define __CMP__

#include <unistd.h>
#include<seal/seal.h>
#include <cassert>
#include <iomanip>
#include <vector>
using namespace std;
using namespace seal;

Ciphertext cdcmp(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, int n,seal::Plaintext a, seal::Ciphertext b);
vector<uint64_t> cdcmp_encode_b(vector<uint64_t> b,int n, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row);
vector<uint64_t> cdcmp_encode_a(vector<uint64_t> a,int n, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row);
vector<uint64_t> cdcmp_decode_a_gt_b_dec(Ciphertext cipher_result, Decryptor *decryptor, BatchEncoder *batch_encoder, uint64_t num, uint64_t num_slots_per_element, uint64_t num_cmps_per_row, uint64_t row_count);
EncryptionParameters cdcmp_init(int n);

Ciphertext rdcmp(Evaluator *evaluator,RelinKeys* rlk_server, int n,std::vector<seal::Plaintext> a, std::vector<seal::Ciphertext> b);
vector<vector<uint64_t>> rdcmp_encode_a(vector<uint64_t> a,int n, uint64_t slot_count, uint64_t row_count);
vector<vector<uint64_t>> rdcmp_encode_b(vector<uint64_t> b,int n, uint64_t slot_count, uint64_t row_count);
EncryptionParameters rdcmp_init(int n);

Ciphertext tecmp_norm(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, vector<uint64_t> a,vector<Ciphertext> b, int l,int m,uint64_t m_degree, seal::Ciphertext one_zero_init_cipher);
Ciphertext tecmp(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, vector<uint64_t> a,vector<Ciphertext> b, int l,int m,uint64_t m_degree, seal::Ciphertext one_zero_init_cipher);
vector<Ciphertext> tecmp_encode_b_enc(vector<uint64_t> encrypted_op, seal::BatchEncoder *batch_encoder, seal::Encryptor *encryptor, int l, int m,int slot_count, int row_count, int m_degree, int num_cmps_per_row, int num_slots_per_element);
vector<vector<uint64_t>> tecmp_encode_b(vector<uint64_t> b, int l, int m,int m_degree,uint64_t num_cmps);
vector<Ciphertext> tecmp_thermometer_encode_b_enc(vector<vector<uint64_t>> encrypted_op_encode, seal::BatchEncoder *batch_encoder, seal::Encryptor *encryptor, int l, int m,int slot_count, int row_count, int m_degree, int num_cmps_per_row, int num_slots_per_element);
vector<uint64_t> tecmp_encode_a(uint64_t a, int l, int m, uint64_t m_degree);
vector<uint64_t> tecmp_decode_a_gt_b_dec(Ciphertext cipher_result, Decryptor *decryptor, BatchEncoder *batch_encoder, uint64_t num, uint64_t num_slots_per_element, uint64_t num_cmps_per_row, uint64_t row_count);
EncryptionParameters tecmp_norm_init(int n,int l,int m);
EncryptionParameters tecmp_init(int n,int l,int m);
Ciphertext clear_cipher_result( Evaluator *evaluator, RelinKeys *rlk_server, Ciphertext result, Plaintext one_zero );
Plaintext init_one_zero_salt(BatchEncoder *batch_encoder, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count);


#endif