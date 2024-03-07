

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
