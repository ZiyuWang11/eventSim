// lut.cpp - lookup table methods

#include <iostream>
#include "lut.h"

// Default constructor
LUT::LUT() : lutSize_(256), lutNum_(1)
{
    std::cout << "Default Constructor" << std::endl;
    lutTable_ = new int[lutSize_];
}

// Constructor
LUT::LUT(size_t lutSize, size_t lutNum) : lutSize_(lutSize), lutNum_(lutNum)
{
    std::cout << "Constructor" << std::endl;
    lutTable_ = new int[lutSize_];

    // change initialization method based on NN
    for (size_t i = 0; i < lutSize_; ++i) {
        lutTable_[i] = 255 - i;
    }

    // std::cout << "Print LUT" << std::endl;
    // for (size_t i = 0; i < lutSize_; ++i) {
    //     std::cout << "Entry " << (int)i << ": " << (int)lutTable_[i] << std::endl;
    // }

}

int* LUT::compute(float* dataIn) const
{
    // data conversion to 8-bit
    static int result[16];
    for (size_t i = 0; i < lutNum_; ++i) {
        // change scaling method based on NN
        int data8bit = 8 * dataIn[i] + 20;
        data8bit = data8bit > 255 ? 255 : data8bit;
        data8bit = data8bit < 0 ? 0 : data8bit;
        result[i] = lutTable_[data8bit];
    }

    return result;
}

// Destructor
LUT::~LUT()
{
    std::cout << "Destructor" << std::endl;
    delete[] lutTable_;
}
