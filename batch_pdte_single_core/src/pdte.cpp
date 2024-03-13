#include "pdte.h"


Plaintext init_b_zero_salt(BatchEncoder *batch_encoder,int b, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count){
    vector<uint64_t> one_zero(slot_count, 0ULL);
    for(int j = 0; j < num_cmps ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        one_zero[jdx] = b;
    }
    Plaintext pt; 
    batch_encoder->encode(one_zero, pt);
    return pt;
}

Plaintext init_one(BatchEncoder *batch_encoder,uint64_t slot_count){
    vector<uint64_t> one_vector(slot_count, 1ULL);
    Plaintext one;
    batch_encoder->encode(one_vector, one);
    return one;
}

Ciphertext init_cipher_zero(BatchEncoder *batch_encoder, Encryptor *encryptor,uint64_t slot_count){
    vector<uint64_t> zero_vector(slot_count, 0ULL);
    Plaintext zero;
    batch_encoder->encode(zero_vector, zero);
    Ciphertext cipher_zero;
    encryptor->encrypt(zero, cipher_zero);
    return cipher_zero;
}


Plaintext init_salt(BatchEncoder *batch_encoder, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count, std::uniform_int_distribution<>& distrib,std::mt19937& gen){

    vector<uint64_t> salt(slot_count, 0ULL);
    for(int j = 0; j < num_cmps ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        salt[jdx] = distrib(gen);
    }
    Plaintext pt; 
    batch_encoder->encode(salt, pt);
    return pt;
}

Plaintext init_salt_full(BatchEncoder *batch_encoder, int slot_count, int num_cmps, std::uniform_int_distribution<>& distrib,std::mt19937& gen){

    vector<uint64_t> salt(slot_count, 0ULL);
    for(int j = 0; j < num_cmps ; j++){
        salt[j] = distrib(gen);
    }
    Plaintext pt; 
    batch_encoder->encode(salt, pt);
    return pt;
}


vector<vector<Ciphertext>> rdcmp_pdte_clear_line_relation(BatchEncoder *batch_encoder, Evaluator *evaluator, vector<vector<Ciphertext>> out,int leaf_num,int data_m,uint64_t slot_count){
    srand((unsigned)time(NULL));
    vector<uint64_t> leaf_num_permutation = random_permutation(leaf_num); //0 1 2 3 ... leaf_num
    vector<Ciphertext> x(0);
    vector<Ciphertext> y(0);
    for(int i = 0; i < leaf_num; i++){
        x.push_back(out[0][leaf_num_permutation[i]]);
        y.push_back(out[1][leaf_num_permutation[i]]);
    }
    int log_data_m = log(data_m + 1)/log(2);
    cout<<"log_data_m " <<log_data_m<<endl;
    for(int i = 0;i < log_data_m; i++){
        vector<uint64_t> data_m_permutation = random_permutation(data_m);
        vector<uint64_t> W0(slot_count,0);
        vector<uint64_t> W1(slot_count,0);
        int data_m_half = data_m / 2;
        for(int j = 0; j < data_m_half;j++){
            W0[j] = 1;
            W1[data_m_half + j] = 1;
        }
        bool flag_data_m_odd = data_m % 2;
        if(flag_data_m_odd){
            W1[data_m - 1] = 1;
        }
        Plaintext W0_pt, W1_pt;
        batch_encoder->encode(W0, W0_pt);
        batch_encoder->encode(W1, W1_pt);
        vector<uint64_t> L = random_permutation(leaf_num);
        vector<Ciphertext> x_temp0(0);
        vector<Ciphertext> y_temp0(0);
        vector<Ciphertext> x_temp1(0);
        vector<Ciphertext> y_temp1(0);
        for(int j = 0; j < leaf_num; j++){
            x_temp0.push_back(x[j]);
            y_temp0.push_back(y[j]);
            x_temp1.push_back(x[j]);
            y_temp1.push_back(y[j]);
        }
        for(int j = 0;j < leaf_num; j++){
            evaluator->multiply_plain_inplace(x_temp0[j],W0_pt);
            evaluator->multiply_plain_inplace(x_temp1[j],W1_pt);
            evaluator->multiply_plain_inplace(y_temp0[j],W0_pt);
            evaluator->multiply_plain_inplace(y_temp1[j],W1_pt);
        }
        for(int j = 0;j < leaf_num; j++){
            evaluator->add(x_temp0[L[j]],x_temp1[j],x[j]);
            evaluator->add(y_temp0[L[j]],y_temp1[j],y[j]);
        }
    }
    out[0] = x;
    out[1] = y;
    return out;
}

vector<vector<Ciphertext>> tecmp_cdcmp_pdte_clear_line_relation(BatchEncoder *batch_encoder, Evaluator *evaluator, vector<vector<Ciphertext>> out,int leaf_num,int data_m,uint64_t slot_count, uint64_t row_count, uint64_t num_cmps_per_row, uint64_t num_slots_per_element){
    srand((unsigned)time(NULL));
    vector<uint64_t> leaf_num_permutation = random_permutation(leaf_num); //0 1 2 3 ... leaf_num
    vector<Ciphertext> x(0);
    vector<Ciphertext> y(0);
    for(int i = 0; i < leaf_num; i++){
        x.push_back(out[0][leaf_num_permutation[i]]);
        y.push_back(out[1][leaf_num_permutation[i]]);
    }

    int log_data_m = log(data_m + 1)/log(2);

    //cout<<"data_m " <<data_m<<endl;
    cout<<"log_data_m " <<log_data_m<<endl;
    for(int i = 0;i < log_data_m; i++){
        vector<uint64_t> data_m_permutation = random_permutation(data_m);
        vector<uint64_t> W0(slot_count,0);
        vector<uint64_t> W1(slot_count,0);
        int data_m_half = data_m / 2;
        //cout<<"data_m_half "<<data_m_half<<endl;

        for(int j = 0; j < data_m_half;j++){
            bool flag = data_m_permutation[j] < num_cmps_per_row; 
            uint64_t jdx = flag ? ( data_m_permutation[j] * num_slots_per_element ) : ( row_count + (data_m_permutation[j] - num_cmps_per_row) * num_slots_per_element);
            W0[jdx] = 1;
            bool flag2 = data_m_permutation[data_m_half + j] < num_cmps_per_row; 
            uint64_t jdx2 = flag2 ? ( data_m_permutation[data_m_half + j] * num_slots_per_element ) : ( row_count + (data_m_permutation[data_m_half + j] - num_cmps_per_row) * num_slots_per_element);
            W1[jdx2] = 1;
        }

        bool flag_data_m_odd = data_m % 2;

        //cout<<"flag_data_m_odd "<<flag_data_m_odd<<endl;
        if(flag_data_m_odd){
            bool flag2 = data_m_permutation[data_m - 1] < num_cmps_per_row; 
            uint64_t jdx2 = flag2 ? ( data_m_permutation[data_m - 1] * num_slots_per_element ) : ( row_count + (data_m_permutation[data_m - 1] - num_cmps_per_row) * num_slots_per_element);
            W1[jdx2] = 1;
        }

        Plaintext W0_pt, W1_pt;
        batch_encoder->encode(W0, W0_pt);
        batch_encoder->encode(W1, W1_pt);
        vector<uint64_t> L = random_permutation(leaf_num);
        vector<Ciphertext> x_temp0(0);
        vector<Ciphertext> y_temp0(0);
        vector<Ciphertext> x_temp1(0);
        vector<Ciphertext> y_temp1(0);
        for(int j = 0; j < leaf_num; j++){
            x_temp0.push_back(x[j]);
            y_temp0.push_back(y[j]);
            x_temp1.push_back(x[j]);
            y_temp1.push_back(y[j]);
        }
        for(int j = 0;j < leaf_num; j++){
            evaluator->multiply_plain_inplace(x_temp0[j],W0_pt);
            evaluator->multiply_plain_inplace(x_temp1[j],W1_pt);
            evaluator->multiply_plain_inplace(y_temp0[j],W0_pt);
            evaluator->multiply_plain_inplace(y_temp1[j],W1_pt);
        }
        for(int j = 0;j < leaf_num; j++){
            evaluator->add(x_temp0[L[j]],x_temp1[j],x[j]);
            evaluator->add(y_temp0[L[j]],y_temp1[j],y[j]);
        }
    }
    out[0] = x;
    out[1] = y;
    return out;
}

