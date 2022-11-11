// lut.h - header file of lookup table
#ifndef LUT_H_
#define LUT_H_

#include <iostream>

class LUT
{
private:
    const size_t lutSize_;
    // number of data to get from tile
    const size_t lutNum_;
    int* lutTable_;

public:
    // Default constructor
    LUT();

    // Constructor - initialize LUT
    LUT(size_t lutSize = 256, size_t lutNum = 1);

    // Get data from tile and conversion
    int* compute(float* dataIn) const;

    // Destructor
    ~LUT();
};

#endif // LUT_H_
