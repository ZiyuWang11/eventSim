// lut.cpp - lookup table methods

#include <cmath>
#include <iostream>
#include <vector>
#include "lut.h"

extern const int dataPrecision;
extern const int busWidth;

// Default constructor
LUT::LUT() : lutNum_(256), lutSize_(1)
{
    std::cout << "Default Constructor" << std::endl;
    lutTable_ = new int[lutSize_];
}

// Constructor
LUT::LUT(size_t outNum, int lutNum, std::string af) : outNum_(outNum), lutNum_(lutNum), activationFunction_(af), lutSize_((int) pow(2, dataPrecision))
{
    // std::cout << "Constructor" << std::endl;
   
    // LUT size depends on the data precision
    // lutSize_ = (int) pow(2, dataPrecision);
    lutTable_ = new int[lutSize_];

    // change initialization method based on NN
    for (int i = 0; i < lutSize_; ++i) {
        // Consider positive/negative outputs, index only positive
        lutTable_[i] = (int) (i - pow(2, dataPrecision - 1));
        
        if (activationFunction_ == "ReLU") {
            if (lutTable_[i] <= 0) {
                lutTable_[i] = 0;
            }
            else {
                lutTable_[i] = lutTable_[i];
            }
        } 
        
        else if(activationFunction_ == "sigmoid") {
            lutTable_[i] = (int) (1 / (1 + exp(-lutTable_[i])));    
        }
    }

    // Calculate output latency based number of outputs and LUT numbers
    if (dataPrecision * lutNum_ < busWidth) { // constraind by LUT numbers
        outLatency_ = (int) ((outNum_ - 1) / lutNum_ + 1);
    }
    else if (dataPrecision * lutNum_ < busWidth) { // constraind by bus
        // clock cycles to convert one data conversion
        int cycle4conversion = (int) ((busWidth - 1) / (dataPrecision * lutNum_) + 1);
        outLatency_ = (int) ((outNum_ - 1) / lutNum_ + 1) * cycle4conversion; 
    }

    // std::cout << "Print LUT" << std::endl;
    // for (size_t i = 0; i < lutSize_; ++i) {
    //     std::cout << "Entry " << (int)i << ": " << (int)lutTable_[i] << std::endl;
    // }
    
    printf("--------------------\n");
    printf("LUT Size: %d\n", lutSize_);
    printf("Number of LUT: %d\n", lutNum_);
}

// Get Latency for coversion and forwording all output data from the tile
int LUT::getTime() const
{
    return outLatency_;
}

// for convenience, change to a vector version
// int* LUT::compute(float* dataIn) const
// {
    // data conversion to 8-bit
//     static int result[16];
//     for (size_t i = 0; i < lutNum_; ++i) {
        // change scaling method based on NN
//         int data8bit = 8 * dataIn[i] + 20;
//         data8bit = data8bit > 255 ? 255 : data8bit;
//         data8bit = data8bit < 0 ? 0 : data8bit;
//         result[i] = lutTable_[data8bit];
//     }

//     return result;
// }

std::vector<int> LUT::compute(std::vector<int> dataIn) const
{
    // Initialize a vector for the output data
    int N = dataIn.size(); // size for input and output vector
    std::vector<int> dataOut(N, 0);
    // Vector are scaled in tile in the range of [-2^N, 2^N-1], N is precision
    for (int i = 0; i < N; ++i) {
        int index = dataIn[i] + (int) pow(2, dataPrecision - 1);
        index = index < 0 ? 0 : index;
        index = index > lutSize_ ? lutSize_ : index;
        dataOut[i] = lutTable_[index];
    }

    return dataOut;
}

// Debug Methods
void LUT::showLUT() const
{
    std::cout << "Print LUT" << std::endl;
    for (int i = 0; i < lutSize_; ++i) {
        std::cout << "Entry " << (int)i << ": " << (int)lutTable_[i] << std::endl;
    }
}

// Destructor
LUT::~LUT()
{
    // std::cout << "Destructor" << std::endl;
    delete[] lutTable_;
}
