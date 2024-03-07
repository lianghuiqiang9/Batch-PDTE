#include "utils.h"

using namespace std;

void swap(vector<int>& v, int i, int j)
{
    int tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
}

int BinaryVectorTointeger(const std::vector<int>& bits)
{
    int pow2 = 1;
    int res = 0;
    for (int i = 0; i < bits.size(); i++){
        res += bits[i] * pow2;
        pow2 <<=1;
    }
    return res;
}

std::vector<int> IntegerToBinaryVector(int num, int num_bits) {
    std::vector<int> binaryVector(num_bits);
        for(int i = 0; i < num_bits; i++){
        binaryVector[i] = num & 1;
        num >>= 1;
    }
    return binaryVector;
}

int bitLength(int num) {
    int length = 0;
    while (num > 0) {
        num >>= 1; 
        length++;
    }
    return length;
}

void print_data(std::vector<std::vector<uint64_t>> data){
        for (const auto& row : data) {
        for (const auto& cell : row) {
            std::cout << cell << ",";
        }
        std::cout << std::endl;
    }
}
std::vector<std::vector<uint64_t>> Transpose(std::vector<std::vector<uint64_t>> A){
    vector<vector<uint64_t>> B;
    for(int i=0;i<A[0].size();i++){
        vector<uint64_t> temp(A.size(), 0ULL);
        for(int j=0;j<A.size();j++){
            temp[j]=A[j][i];
        }
        B.push_back(temp);
    }
    return B;
}

std::vector<std::vector<uint64_t>> Matrix_padding(std::vector<std::vector<uint64_t>> A, int slot_count){
    vector<vector<uint64_t>> B;
    for(int i=0;i<A.size();i++){
        vector<uint64_t> temp(slot_count, 0ULL);
        for(int j=0;j<A[0].size();j++){
            temp[j]=A[i][j];
        }
        for(int j=A[0].size();j<slot_count ;j++){
            temp[j] = 0ULL;
        }
        B.push_back(temp);
    }
    return B;
}


std::vector<uint64_t> random_permutation(int n)
{
	int x;
	std::vector<uint64_t> a(n);
	if (n <= 0) { cout<<"random_permutation error"<<endl; exit(-1); }
	for (int i = 0;i < n;i++)
	{
		x = rand() % (n);
		for (int j = 0;j < i;j++)
		{
			if (x == a[j])
			{
				x = rand() % (n);
				j = -1;
			}
		}
		a[i] = x;
	}
    return a;
}

