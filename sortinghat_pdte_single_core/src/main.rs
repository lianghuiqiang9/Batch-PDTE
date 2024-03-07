use std::fs;
use std::path::Path;


use sortinghat::decision_tree::*;
use sortinghat::Scalar;
use std::time::Instant;
use sortinghat::Context;
use sortinghat::rlwe::*;
use std::env;
use concrete_core::backends::core::private::crypto::bootstrap::FourierBuffers;


fn parse_csv(path: &Path) -> Vec<Vec<usize>> {
    let x_test_f = fs::File::open(path).expect("csv file not found, consider using --artificial");

    let mut x_test: Vec<Vec<usize>> = vec![];
    let mut x_train_rdr = csv::Reader::from_reader(x_test_f);//逐行读取，然后res为某一行。
    for res in x_train_rdr.records() {
        let record = res.unwrap();//读取当前行的内容，并将其存储在 record 中。unwrap() 用于处理可能的错误。
        let row = record.iter().map(|s| {
            s.parse().unwrap()//将字符串变为usize
        }).collect();
        x_test.push(row);
    }

    x_test
}

fn main() {

    let args: Vec<String> = env::args().collect();
    let dir_data:String  = args[1].parse().expect("Invalid number");
    let input_size: usize = args[2].parse().expect("Invalid number");
    println!("{},{}",dir_data,input_size);
    println!("client load private data");

    let base_path = Path::new(&dir_data);
    let x_test_path = base_path.join("x_test.csv");
    let x_test = parse_csv(&x_test_path);

    println!("client print data");
    for i in 0..input_size{println!("Print x_test[{}]: {:?}",i,x_test[i]);}

    let x_test_input: Vec<Vec<usize>>=x_test.into_iter().take(input_size).collect();
    println!("Print x_test_input.len():{:?}",x_test_input.len());

    println!("client generater the parameters..");
    let setup_instant_global = Instant::now();
    let setup_instant = Instant::now();

    let mut ctx_client = Context::default();

    println!("Print ctx:{}",ctx_client);

    let sk = ctx_client.gen_rlwe_sk();

    let neg_sk_ct = sk.neg_gsw(&mut ctx_client);//RGSW(-s)

    let mut buffers_client = ctx_client.gen_fourier_buffers();
    let ksk_map = gen_all_subs_ksk_fourier(&sk, &mut ctx_client, &mut buffers_client);

    println!("client generate sk, RGSW(-s), ksk_map time : {:?}", setup_instant.elapsed());
    let setup_instant = Instant::now();

    println!("client encrypt the private data");
    let client_cts: Vec<Vec<Vec<RLWECiphertext>>> = x_test_input.iter().map(|f| encrypt_feature_vector(&sk, &f, &mut ctx_client)).collect();

    println!("client encrypt the data time : {:?}", setup_instant.elapsed());
    let setup_instant = Instant::now();
    println!("server load decision tree..");

    let dir_tree = dir_data;
    let base_path_tree = Path::new(& dir_tree);
    let model_path = base_path_tree.join("model.json");
    let model_f = fs::File::open(model_path).unwrap();
    let root: Node = serde_json::from_reader(model_f).expect("cannot parse json");
    println!("server load decision tree time : {:?}", setup_instant.elapsed());


    println!("server print decision tree");
    //println!("Print root:{:?}",root);
    println!("count depth = {}",root.count_depth());

    println!("server generater the parameters..");
    let ctx_server = Context::default();//生成默认参数
    let mut buffers_server = ctx_server.gen_fourier_buffers();

    println!("server start to private transform..");
    let setup_instant = Instant::now();

    let flat_nodes = root.flatten();

    let server_f = |ct, buffers_server: &mut FourierBuffers<Scalar>| {
        println!("server_f");
        let enc_root = {
            let mut rgsw_cts = compare_expand(&flat_nodes, ct, &neg_sk_ct, &ksk_map, &ctx_server, buffers_server);
            EncNode::new(&root, &mut rgsw_cts)
        };
        let final_label_ct = enc_root.eval(&ctx_server, buffers_server);
        final_label_ct
    };

    let output_cts: Vec<Vec<RLWECiphertext>> =client_cts.iter().map(|ct| {
        server_f(ct, &mut buffers_server)
    }).collect();
    println!("server evaluate {} line data, finish step 2 time: {:?} ",input_size, setup_instant.elapsed());
    
    let average_time =setup_instant.elapsed()/input_size.try_into().unwrap();
    println!("server evalutae {} line data, average finish step 2 time: {:?} ",input_size, average_time);

    println!("client decrypt the output");
    let setup_instant = Instant::now();

    let mut predictions = vec![];
        for (ct, feature) in output_cts.iter().zip(x_test_input.iter()) {
                    let expected_scalar = root.eval(feature) as Scalar;
                    //println!("expected_scalar:{:?}",expected_scalar);
                    let actual_scalar = decrypt_and_recompose(&sk, ct, &ctx_client);
                    //println!("actual_scalar:{:?}",actual_scalar);
                    assert_eq!(expected_scalar, actual_scalar);
                    predictions.push(expected_scalar);
                }        
    println!("server decrypt {} line data, the output time: {:?}",input_size, setup_instant.elapsed());            
    println!("overall time: {:?}", setup_instant_global.elapsed());

    let ciphertext_size = 88;

    println!("Single line time   : {:?}", average_time);
    println!("Single line comun. : {} KB",(client_cts[0].len()*7 + output_cts[0].len())*ciphertext_size);
}
