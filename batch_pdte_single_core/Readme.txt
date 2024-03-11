
###

## cmp_bench
cd ours_pdte_single_core

cd cmp_bench
mkdir build
cd build
cmake ..
make

./tecmp -l 4 -m 2
./tecmp -l 4 -m 3
./tecmp -l 8 -m 2
./tecmp -l 16 -m 2
./tecmp -l 16 -m 3
./tecmp -l 32 -m 2
./tecmp -l 64 -m 2
./tecmp -l 128 -m 2
./tecmp -l 256 -m 2
./tecmp -l 512 -m 2

#the most bit precision
./tecmp -l 2048 -m 13


./lrcmp -n 8
./lrcmp -n 16
./lrcmp -n 16
./lrcmp -n 32
./lrcmp -n 64
./lrcmp -n 64
./lrcmp -n 128
./lrcmp -n 256
./lrcmp -n 512
./lrcmp -n 1024

./rrcmp -n 8
./rrcmp -n 16
./rrcmp -n 16
./rrcmp -n 32
./rrcmp -n 64
./rrcmp -n 64
./rrcmp -n 128
./rrcmp -n 256
./rrcmp -n 512
./rrcmp -n 1024


## pdte

cd ..
cd ..
mkdir build
cd build
cmake ..
make

#no clr process 

./tecmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 1024 -l 4 -m 3 -c 0
./tecmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1024 -l 4 -m 3 -c 0
./tecmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 1024 -l 4 -m 3 -c 0
./tecmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1024 -l 4 -m 3 -c 0

./lrcmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 16383 -n 16 -c 0
./lrcmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 16383 -n 16 -c 0
./lrcmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 16383 -n 16 -c 0
./lrcmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 16383 -n 16 -c 0


./rrcmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 1024 -n 16 -c 0
./rrcmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1024 -n 16 -c 0
./rrcmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 1024 -n 16 -c 0
./rrcmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1024 -n 16 -c 0


#have clr process

./tecmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1
./tecmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1
./tecmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 2048 -l 4 -m 3 -c 1
./tecmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 2048 -l 4 -m 3 -c 1


./lrcmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 512 -n 16 -c 1
./lrcmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 512 -n 16 -c 1
./lrcmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 512 -n 16 -c 1
./lrcmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 512 -n 16 -c 1


./rrcmp_pdte -t ../data/heart_11bits/model.json -v ../data/heart_11bits/x_test.csv -r 256 -n 16 -c 1
./rrcmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 256 -n 16 -c 1
./rrcmp_pdte -t ../data/spam_11bits/model.json -v ../data/spam_11bits/x_test.csv -r 256 -n 16 -c 1
./rrcmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 256 -n 16 -c 1


#high-precision in single row

tecmp-pdte
./tecmp_pdte -t ../data/breast_11bits/model.json -v ../data/breast_11bits/x_test.csv -r 1 -l 1 -m 12 -c 0
./tecmp_pdte -t ../data/breast_16bits/model.json -v ../data/breast_16bits/x_test.csv -r 1 -l 2 -m 12 -c 0
./tecmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 4 -m 12 -c 0
./tecmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 8 -m 12 -c 0
./tecmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -l 16 -m 12 -c 0

rrcmp-pdte

./rrcmp_pdte -t ../data/breast_16bits/model.json -v ../data/breast_16bits/x_test.csv -r 1 -n 16 -c 0
./rrcmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 32 -c 0
./rrcmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 64 -c 0
./rrcmp_pdte -t ../data/breast_31bits/model.json -v ../data/breast_31bits/x_test.csv -r 1 -n 128 -c 0


#single line in different tree
./tecmp_pdte -t ../data/breast_31bits/model.json -v ../data/spam_11bits/x_test.csv -r 1 -l 3 -m 11 -c 0
./tecmp_pdte -t ../data/electricity_10bits/model.json -v ../data/electricity_10bits/x_test.csv -r 1 -l 3 -m 11 -c 0
//29343.2 ms 489541 KB
