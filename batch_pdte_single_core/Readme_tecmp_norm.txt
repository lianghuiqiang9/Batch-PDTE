
#base
./tecmp_norm -l 4 -m 2
./tecmp_norm -l 3 -m 4
./tecmp_norm -l 8 -m 2
./tecmp_norm -l 8 -m 4
./tecmp_norm -l 6 -m 6
./tecmp_norm -l 8 -m 8
./tecmp_norm -l 10 -m 12
./tecmp_norm -l 11 -m 12


#no clr process 

./tecmp_norm_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 512 -l 3 -m 4 -c 0
./tecmp_norm_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 512 -l 3 -m 4 -c 0
./tecmp_norm_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 512 -l 3 -m 4 -c 0
./tecmp_norm_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 512 -l 3 -m 4 -c 0

#have clr process

./tecmp_norm_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 1024 -l 3 -m 4 -c 1
./tecmp_norm_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1024 -l 3 -m 4 -c 1
./tecmp_norm_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 1024 -l 3 -m 4 -c 1
./tecmp_norm_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1024 -l 3 -m 4 -c 1

#high-precision in single row

tecmp_norm_pdte-pdte

./tecmp_norm_pdte -t ../data/breast_16bits/model.json -v ../data/breast_16bits/x_test.csv -r 1 -l 2 -m 8 -c 0
./tecmp_norm_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 3 -m 11 -c 0
./tecmp_norm_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 6 -m 11 -c 0
./tecmp_norm_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 11 -m 12 -c 0

#single line in different tree
./tecmp_norm_pdte -t ../data/breast_31bits/model.json -v ../data/spam_11bits/x_test.csv -r 1 -l 3 -m 11 -c 0
./tecmp_norm_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1 -l 3 -m 11 -c 0

