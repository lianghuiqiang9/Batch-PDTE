
###

## cmp_bench
cd level_up_pdte_single_core

cd cmp_bench
g++ -o cmp -O3 cmp_bench.cpp utils.cpp -I /usr/local/include/SEAL-4.1 -lseal-4.1

./cmp -n 8
./cmp -n 12
./cmp -n 16
./cmp -n 32
./cmp -n 36
./cmp -n 64
./cmp -n 128
./cmp -n 256
./cmp -n 512
./cmp -n 1024

## pdte

cd .. 
cd ./src

#rcc & folklore

mkdir build
cd build
cmake ..
make

#data test
#-e 1 FOLKLORE

./main  -t ../../data/heart_11bits/model.json -v ../../data/heart_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 0 -e 1
./main  -t ../../data/breast_11bits/model.json -v ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 0 -e 1
./main  -t ../../data/spam_11bits/model.json -v ../../data/spam_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 0 -e 1
./main  -t ../../data/electricity_10bits/model.json -v ../../data/electricity_10bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 10 -w 0 -e 1
./main  -t ../../data/steel_11bits/model.json -v ../../data/steel_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 0 -e 1

#-e 0 RCC

./main  -t ../../data/heart_11bits/model.json -v ../../data/heart_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 4 -e 0
./main  -t ../../data/breast_11bits/model.json -v ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 4 -e 0
./main  -t ../../data/spam_11bits/model.json -v ../../data/spam_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 4 -e 0
./main  -t ../../data/electricity_10bits/model.json -v ../../data/electricity_10bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 10 -w 4 -e 0
./main  -t ../../data/steel_11bits/model.json -v ../../data/steel_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 11 -w 4 -e 0


#high precision 

#rcc

./main  -t ../../data/breast_11bits/model.json -v ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 12 -w 2 -e 0
./main  -t ../../data/breast_16bits/model.json -v ../../data/breast_16bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 16 -w 4 -e 0
./main  -t ../../data/breast_16bits/model.json -v ../../data/breast_16bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 24 -w 4 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 32 -w 4 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 48 -w 8 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 64 -w 16 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 92 -w 16 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 128 -w 16 -e 0
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 184 -w 16 -e 0

#folklore

./main  -t ../../data/breast_11bits/model.json -v ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 12 -w 0 -e 1
./main  -t ../../data/breast_16bits/model.json -v ../../data/breast_16bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 16 -w 0 -e 1
./main  -t ../../data/breast_16bits/model.json -v ../../data/breast_16bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 24 -w 0 -e 1
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 32 -w 0 -e 1
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 48 -w 0 -e 1
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 64 -w 0 -e 1
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 96 -w 0 -e 1
./main  -t ../../data/breast_31bits/model.json -v ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/breast/ -n 128 -w 0 -e 1


cd ..
cd ..
cd src2

mkdir build
cd build
cmake ..
make

#xxcmp

#data test
./main -v -m ../../data/heart_11bits/model.json -a ../../data/heart_11bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 11
./main -v -m ../../data/breast_11bits/model.json -a ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 11
./main -v -m ../../data/spam_11bits/model.json -a ../../data/spam_11bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 11
./main -v -m ../../data/electricity_10bits/model.json -a ../../data/electricity_10bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 10

#high precision 
./main -v -m ../../data/breast_11bits/model.json -a ../../data/breast_11bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 12
./main -v -m ../../data/breast_16bits/model.json -a ../../data/breast_16bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 24
./main -v -m ../../data/breast_31bits/model.json -a ../../data/breast_31bits/x_test.csv -s ../../experiments/results-v9/src1/heart/ -n 48










