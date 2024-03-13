#include "cmp.h"



//a>b
Ciphertext cdcmp(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, int n,seal::Plaintext a, seal::Ciphertext b){
    Ciphertext eq;
    Ciphertext gt;
    evaluator->multiply_plain(b,a,gt);
    evaluator->add(gt,gt,eq);
    evaluator->negate_inplace(eq);
    evaluator->add_inplace(eq,b);
    evaluator->add_plain_inplace(eq,a);

    int depth = log(n)/log(2);
    for(int i = 0; i < depth ; i++){
        int temp1 = 1<<i;
        //cout<<" temp1 : "<<temp1<<endl;
        Ciphertext gt_temp;
        Ciphertext eq_temp;
        evaluator->rotate_rows(gt, temp1, *gal_keys_server, gt_temp);
        evaluator->rotate_rows(eq, temp1, *gal_keys_server, eq_temp);
        evaluator->multiply_inplace(eq,eq_temp);
        evaluator->relinearize_inplace(eq,*rlk_server);
        evaluator->multiply_inplace(gt,eq_temp);
        evaluator->relinearize_inplace(gt,*rlk_server);
        evaluator->add_inplace(gt, gt_temp);
    }
    return gt;
}

//1-b
vector<uint64_t> cdcmp_encode_b(vector<uint64_t> b,int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row){
    vector<uint64_t> b_v(slot_count, 0ULL);
        for(int j = 0; j < num_cmps_per_row; j++){
            for(int i = 0 ; i < num_slots_per_element; i++){ 
                b_v[ i + j * num_slots_per_element ] = 1 - (b[j] >> i ) & 1;  
                b_v[ row_count + i + j * num_slots_per_element]= 1 - (b[num_cmps_per_row + j] >> i ) & 1;

            }
        }
        //b_v[slot_count-1] = 3;
    return b_v;
}

//a
vector<uint64_t> cdcmp_encode_a(vector<uint64_t> a,int num_slots_per_element, uint64_t slot_count,uint64_t row_count, uint64_t num_cmps_per_row){
    vector<uint64_t> a_v(slot_count, 0ULL);
    for(int j = 0; j < num_cmps_per_row; j++){
        for(int i = 0 ; i < num_slots_per_element; i++){ 
            a_v[ i + j * num_slots_per_element ] = (a[j] >> i ) & 1;  
            a_v[ row_count + i + j * num_slots_per_element]= (a[num_cmps_per_row + j] >> i ) & 1;
        }
    }
    return a_v;
}


Ciphertext clear_cipher_result( Evaluator *evaluator, RelinKeys *rlk_server, Ciphertext result, Plaintext one_zero ){
    Ciphertext ans;
    evaluator->multiply_plain(result, one_zero, ans);
    evaluator->relinearize_inplace(result,*rlk_server);
    return ans;
}

vector<uint64_t> cdcmp_decode_a_gt_b_dec(Ciphertext cipher_result, Decryptor *decryptor, BatchEncoder *batch_encoder, uint64_t num, uint64_t num_slots_per_element, uint64_t num_cmps_per_row, uint64_t row_count){
    Plaintext pt;
    decryptor->decrypt(cipher_result, pt);
    vector<uint64_t> res;
    batch_encoder->decode(pt, res);
    vector<uint64_t> ans(num);
    for(int j = 0; j < num ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        ans[j] = res[jdx];
    }
    return ans;            
}

EncryptionParameters cdcmp_init(int n){
    int depth_need_min = log(n)/log(2) + 1;
    cout<<"depth_need_min = "<<depth_need_min<<endl;
    uint64_t log_poly_mod_degree; uint64_t prime_bitlength; 
    if(depth_need_min<4){
        log_poly_mod_degree = 13; prime_bitlength = 17;
    }else if(depth_need_min<12){
        log_poly_mod_degree = 14; prime_bitlength = 17;
    }else{
        cout<<"params l is too large, please choose the params again"<<endl;
        exit(0);
    }

    uint64_t poly_modulus_degree=1<<log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    if(depth_need_min<4){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else if(depth_need_min<8){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    }else if (depth_need_min<11){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    }else if (depth_need_min<12){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else{
        cout<<"l is too large, error"<<endl;
        exit(0);
    }



    return parms;
}

//a > E(b)
Ciphertext rdcmp(Evaluator *evaluator,RelinKeys* rlk_server, int n,std::vector<seal::Plaintext> a, std::vector<seal::Ciphertext> b){
    vector<Ciphertext> eq(n);
    vector<Ciphertext> gt(n);

    for(int i=0;i<n;i++){
        evaluator->multiply_plain(b[i],a[i],gt[i]);
        //evaluator->relinearize_inplace(gt[i],*rlk_server);
    }
    for(int i=0;i<n;i++){
        evaluator->add(gt[i],gt[i],eq[i]);
        evaluator->negate_inplace(eq[i]);
        evaluator->add_inplace(eq[i],b[i]);
        evaluator->add_plain_inplace(eq[i],a[i]);
    }
    int depth = log(n)/log(2);
    for(int i = 0; i < depth ; i++){
        int temp1 = 1<<i;
        int temp0 = 1<<(i + 1);
        //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
        for(int j = 0; j < n; j = j + temp0){
            evaluator->multiply_inplace(gt[j],eq[ j + temp1]);
            evaluator->relinearize_inplace(gt[j],*rlk_server);
            evaluator->add_inplace(gt[j], gt[j + temp1]);
            evaluator->multiply_inplace(eq[j],eq[j + temp1]);
            evaluator->relinearize_inplace(eq[j],*rlk_server);
        }   
    }
    return gt[0];
}

//a
vector<vector<uint64_t>> rdcmp_encode_a(vector<uint64_t> a,int n, uint64_t slot_count, uint64_t row_count){
    vector<vector<uint64_t>> a_v;
    for(int i = 0 ; i < n; i++){
        vector<uint64_t> temp(slot_count, 0ULL);
        for(int j = 0; j < slot_count - 1; j++){
            temp[ j ] = (a[j] >> i ) & 1; 
        }
        temp[ slot_count - 1 ]= 13; //Make sure plaintext is not padded with 0 , otherwise transparent error.
        a_v.push_back(temp);
    }
    return a_v;
}

// 1-b
vector<vector<uint64_t>> rdcmp_encode_b(vector<uint64_t> b,int n, uint64_t slot_count, uint64_t row_count){
    vector<vector<uint64_t>> b_v;
    for(int i = 0 ; i < n; i++){
        vector<uint64_t> temp(slot_count, 0ULL);
        for(int j = 0; j < b.size(); j++){
            temp[ j ] = 1 - (b[j] >> i ) & 1; //1 - a 
        }
        temp[ slot_count - 1 ]= 13; //Make sure plaintext is not padded with 0 , otherwise transparent error.
        b_v.push_back(temp);
    }
    return b_v;
}

EncryptionParameters rdcmp_init(int n){
    int depth_need_min = log(n)/log(2) + 1;
    cout<<"depth_need_min = "<<depth_need_min<<endl;
   
    uint64_t log_poly_mod_degree; uint64_t prime_bitlength; 
    if(depth_need_min<4){
        log_poly_mod_degree = 13; prime_bitlength = 17;
    }else if(depth_need_min<12){
        log_poly_mod_degree = 14; prime_bitlength = 17;
    }else{
        cout<<"params l is too large, please choose the params again"<<endl;
        exit(0);
    }

    uint64_t poly_modulus_degree=1<<log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    if(depth_need_min<4){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else if(depth_need_min<8){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    }else if (depth_need_min<11){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    }else if (depth_need_min<12){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else{
        cout<<"l is too large, error"<<endl;
        exit(0);
    }

    return parms;
}

//return a > E(b)
Ciphertext tecmp_norm(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, vector<uint64_t> a,vector<Ciphertext> b, int l,int m,uint64_t m_degree, seal::Ciphertext one_zero_init_cipher){
    vector<Ciphertext> eq(l);
    vector<Ciphertext> gt(l);

    if(l == 1){
        evaluator->rotate_rows(b[0], a[0], *gal_keys_server, gt[0]);
    }else{
        for(int i=0;i<l;i++){
            evaluator->rotate_rows(b[i], a[i], *gal_keys_server, gt[i]);
            if(a[i]<m_degree-1){
                evaluator->rotate_rows(b[i], a[i] + 1, *gal_keys_server, eq[i]);
            }else{
                eq[i] = one_zero_init_cipher;
            }
            evaluator->sub_inplace(eq[i], gt[i]); //eq = eq - gt ;
        }
    }

    // result = b > a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * gt_0))
    // result = b >= a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * (gt_0 + eq_0)))

    Ciphertext comparisonResult = gt[0];

    for(int i = 1;i < l; i++){
        evaluator->multiply_inplace(comparisonResult,eq[i]); //l-1
        evaluator->relinearize_inplace(comparisonResult,*rlk_server);
        evaluator->add_inplace(comparisonResult,gt[i]);
    }
    return comparisonResult;
}

//return a > E(b)
Ciphertext tecmp(Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys* rlk_server, vector<uint64_t> a,vector<Ciphertext> b, int l,int m,uint64_t m_degree, seal::Ciphertext one_zero_init_cipher){
    vector<Ciphertext> eq(l);
    vector<Ciphertext> gt(l);

    if(l == 1){
        evaluator->rotate_rows(b[0], a[0], *gal_keys_server, gt[0]);
        return gt[0];
    }else{
        for(int i=0;i<l;i++){
            evaluator->rotate_rows(b[i], a[i], *gal_keys_server, gt[i]);
            if(a[i]<m_degree-1){
                evaluator->rotate_rows(b[i], a[i] + 1, *gal_keys_server, eq[i]);
            }else{
                eq[i] = one_zero_init_cipher;
            }
            evaluator->sub_inplace(eq[i], gt[i]); //eq = eq - gt ;
        }
    }

    // result = b > a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * gt_0))
    // result = b >= a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * (gt_0 + eq_0)))

    int depth = log(l)/log(2);
    for(int i = 0; i < depth ; i++){
        int temp1 = 1<<i;
        int temp0 = 1<<(i + 1);
        //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
        for(int j = 0; j < l; j = j + temp0){
            evaluator->multiply_inplace(gt[j],eq[ j + temp1]);
            evaluator->relinearize_inplace(gt[j],*rlk_server);
            evaluator->add_inplace(gt[j], gt[j + temp1]);
            evaluator->multiply_inplace(eq[j],eq[j + temp1]);
            evaluator->relinearize_inplace(eq[j],*rlk_server);
        }   
    }
    return gt[0];
}

//a > E(b)
vector<Ciphertext> tecmp_encode_b_enc(vector<uint64_t> encrypted_op, seal::BatchEncoder *batch_encoder, seal::Encryptor *encryptor, int l, int m,int slot_count, int row_count, int m_degree, int num_cmps_per_row, int num_slots_per_element){    
    vector<Ciphertext> client_input(l);
    
        for(int i = 0 ; i < l; i++){
            vector<uint64_t> temp(slot_count, 1ULL);
            for(int j = 0; j < encrypted_op.size(); j++){
                //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
                //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
                bool flag = j < num_cmps_per_row; 
                uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
                uint64_t b = (encrypted_op[j] >> (i * m) ) & ( m_degree - 1 );
                //cout<<"encrypted_op[" <<j<< "]"<<encrypted_op[j]<<" b = "<< b <<endl;
                for(int k = 0; k <= b; k++){
                    temp[jdx + k] = 0ULL;     
                }
            }
            //print_matrix(temp,row_count);
            Plaintext plaintext; 
            batch_encoder->encode(temp, plaintext);
            encryptor->encrypt(plaintext, client_input[i]);
        }

    return client_input;
}

vector<vector<uint64_t>> tecmp_encode_b(vector<uint64_t> b, int l, int m,int m_degree,uint64_t num_cmps){    
    vector<vector<uint64_t>> encrypted_op_encode;
        for(int i = 0 ; i < l; i++){
            vector<uint64_t> encrypted_op_temp(num_cmps, 0);
            for(int j = 0; j < b.size(); j++){
                encrypted_op_temp[j] = (b[j] >> (i * m) ) & ( m_degree - 1 );
            }
            encrypted_op_encode.push_back(encrypted_op_temp);
        }
    return encrypted_op_encode;
}

//a > E(b)
vector<Ciphertext> tecmp_thermometer_encode_b_enc(vector<vector<uint64_t>> encrypted_op_encode, seal::BatchEncoder *batch_encoder, seal::Encryptor *encryptor, int l, int m,int slot_count, int row_count, int m_degree, int num_cmps_per_row, int num_slots_per_element){    
    vector<Ciphertext> client_input(l);
    
        for(int i = 0 ; i < l; i++){
            vector<uint64_t> temp(slot_count, 1ULL);
            for(int j = 0; j < encrypted_op_encode[i].size(); j++){
                //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
                //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
                bool flag = j < num_cmps_per_row; 
                uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
                uint64_t b = encrypted_op_encode[i][j];
                //cout<<"encrypted_op_encode[" <<j<< "]"<<encrypted_op_encode[j]<<" b = "<< b <<endl;
                for(int k = 0; k <= b; k++){
                    temp[jdx + k] = 0ULL;     
                }
            }
            //print_matrix(temp,row_count);
            Plaintext plaintext; 
            batch_encoder->encode(temp, plaintext);
            encryptor->encrypt(plaintext, client_input[i]);
        }

    return client_input;
}

//a > E(b)
vector<uint64_t> tecmp_encode_a(uint64_t a, int l, int m, uint64_t m_degree){
    vector<uint64_t> a_v(l, 0ULL);
    for(int i = 0 ; i < l; i++){
        a_v[ i ] = (a >> (i * m) ) & (m_degree - 1);
    }
    return a_v;
}

vector<uint64_t> tecmp_decode_a_gt_b_dec(Ciphertext cipher_result, Decryptor *decryptor, BatchEncoder *batch_encoder, uint64_t num, uint64_t num_slots_per_element, uint64_t num_cmps_per_row, uint64_t row_count){
    Plaintext pt;
    decryptor->decrypt(cipher_result, pt);
    vector<uint64_t> res;
    batch_encoder->decode(pt, res);
    vector<uint64_t> ans(num);
    for(int j = 0; j < num ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        ans[j] = res[jdx];
    }
    return ans;            
}


EncryptionParameters tecmp_norm_init(int n,int l,int m){
    
    int depth_need_min = l;
    cout<<"depth_need_min "<< depth_need_min<<endl;

    uint64_t log_poly_mod_degree; uint64_t prime_bitlength; 
    if(depth_need_min<4){
        log_poly_mod_degree = 13; prime_bitlength = 17;
    }else if(depth_need_min<12){
        log_poly_mod_degree = 14; prime_bitlength = 17;
    }else{
        cout<<"params depth_need_min is too large, please choose the params again"<<endl;
        exit(0);
    }

    uint64_t poly_modulus_degree=1<<log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    if(depth_need_min<4){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else if(depth_need_min<8){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    }else if (depth_need_min<11){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    }else if (depth_need_min<12){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else{
        cout<<"depth_need_min is too large, error"<<endl;
        exit(0);
    }

    if(m >= (log_poly_mod_degree)){
        cout<<"log_poly_mod_degree :"<<log_poly_mod_degree<<endl;
        cout<<"params m is too large, need < log_poly_mod_degree, please choose the params again"<<endl;
        exit(0);
    }
    
    return parms;
}


EncryptionParameters tecmp_init(int n,int l,int m){
    
    int depth_need_min = log(l)/log(2);
    if(1<<depth_need_min !=l){
        cout<<" l must be 2^x, error"<<endl;
        exit(0);
    }
    cout<<"depth_need_min "<< depth_need_min<<endl;

    uint64_t log_poly_mod_degree; uint64_t prime_bitlength; 
    if(depth_need_min<4){
        log_poly_mod_degree = 13; prime_bitlength = 17;
    }else if(depth_need_min<12){
        log_poly_mod_degree = 14; prime_bitlength = 17;
    }else{
        cout<<"params depth_need_min is too large, please choose the params again"<<endl;
        exit(0);
    }

    uint64_t poly_modulus_degree=1<<log_poly_mod_degree;
    EncryptionParameters parms = EncryptionParameters(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, prime_bitlength));
    
    if(depth_need_min<4){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else if(depth_need_min<8){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 48, 48, 48, 48, 48, 48, 48 }));
    }else if (depth_need_min<11){
        parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60 }));
    }else if (depth_need_min<12){
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    }else{
        cout<<"depth_need_min is too large, error"<<endl;
        exit(0);
    }

    if(m >= (log_poly_mod_degree)){
        cout<<"log_poly_mod_degree :"<<log_poly_mod_degree<<endl;
        cout<<"params m is too large, need < log_poly_mod_degree, please choose the params again"<<endl;
        exit(0);
    }
    
    return parms;
}


Plaintext init_one_zero_salt(BatchEncoder *batch_encoder, int slot_count,uint64_t num_cmps,uint64_t num_cmps_per_row,uint64_t num_slots_per_element,uint64_t row_count){
    vector<uint64_t> one_zero(slot_count, 0ULL);
    for(int j = 0; j < num_cmps ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        one_zero[jdx] = 1ULL;
    }
    
    Plaintext pt; 
    batch_encoder->encode(one_zero, pt);
    return pt;
}