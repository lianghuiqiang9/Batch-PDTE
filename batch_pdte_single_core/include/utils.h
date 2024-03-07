#ifndef __UTILS__
#define __UTILS__

#include <vector>
#include <iostream> 
#include <random>
#include <cassert>
#include <iomanip>

void swap(std::vector<int>& v, int i, int j);

//from low to high
int BinaryVectorTointeger(const std::vector<int>& bits);

//from low to high
std::vector<int> IntegerToBinaryVector(int num, int num_bits); 

int bitLength(int num);

template <typename T>
std::ostream& operator<< (std::ostream &out, std::vector<T> & u) {
    if (0 == u.size())
        return out << "[ ]";
        
    std::cout << "[";
    for (long i = 0; i < u.size()-1; i++)
        out << u[i] << ", ";
    out << u[u.size()-1] << "]";
    return out;
}


template <typename T>
inline void print_matrix(std::vector<T> matrix, std::size_t row_size)
{
    /*
    We're not going to print every column of the matrix (there are 2048). Instead
    print this many slots from beginning and end of the matrix.
    */
    std::size_t print_size = 20;

    std::cout << std::endl;
    std::cout << "    [";
    for (std::size_t i = 0; i < print_size; i++)
    {
        std::cout << std::setw(3) << std::right << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = row_size - print_size; i < row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
    }
    std::cout << "    [";
    for (std::size_t i = row_size; i < row_size + print_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
    }
    std::cout << std::endl;
}

std::vector<std::vector<uint64_t>> Transpose(std::vector<std::vector<uint64_t>> A);
std::vector<std::vector<uint64_t>> Matrix_padding(std::vector<std::vector<uint64_t>> A, int slot_count);
void print_data(std::vector<std::vector<uint64_t>> data);


std::vector<uint64_t> random_permutation(int n);

#endif