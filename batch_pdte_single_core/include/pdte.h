#ifndef __PDTE__
#define __PDTE__

#include "utils.h"
#include <unistd.h>
#include<seal/seal.h>
using namespace seal;
using namespace std;


Plaintext init_b_zero_salt(BatchEncoder *batch_encoder,int b, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count);
Plaintext init_one(BatchEncoder *batch_encoder,uint64_t slot_count);
Ciphertext init_cipher_zero(BatchEncoder *batch_encoder, Encryptor *encryptor,uint64_t slot_count);
Plaintext init_salt(BatchEncoder *batch_encoder, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count, std::uniform_int_distribution<>& distrib,std::mt19937& gen);
Plaintext init_salt_full(BatchEncoder *batch_encoder, int slot_count, int num_cmps, std::uniform_int_distribution<>& distrib,std::mt19937& gen);


vector<vector<Ciphertext>> rdcmp_pdte_clear_line_relation(BatchEncoder *batch_encoder, Evaluator *evaluator, vector<vector<Ciphertext>> out,int leaf_num,int data_m,uint64_t slot_count);
vector<vector<Ciphertext>> tecmp_cdcmp_pdte_clear_line_relation(BatchEncoder *batch_encoder, Evaluator *evaluator, vector<vector<Ciphertext>> out,int leaf_num,int data_m,uint64_t slot_count, uint64_t row_count, uint64_t num_cmps_per_row, uint64_t num_slots_per_element);



#endif