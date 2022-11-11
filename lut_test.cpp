// lut_test.cpp - lookup table test

#include <iostream>
#include "lut.h"

int main() 
{
    LUT lut(256, 4);

    float dataIn[4] = {1.0, 2.0, 150.0, 200.0};
    int* data;
    std::cout << "here!" << std::endl;
    data = lut.compute(dataIn);
    std::cout << "Result: " << data[0] << "  " << data[1] << std::endl;
 
    return 0;
}
