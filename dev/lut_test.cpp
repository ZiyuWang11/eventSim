// lut_test.cpp - lookup table test

#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "lut.h"

const int dataPrecision = 8;
const int busWidth = 64;

int main() 
{
    std::string activationFunction = "ReLU";
    LUT lut(64, 4, activationFunction);
    lut.showLUT();
    int latency = lut.getTime();
    std::cout << "Latency is: " << latency << std::endl; 

    std::cout << "== Start Data Test ==\n";
    for (int i = 0; i < 256; ++i) {
        std::vector<int> dataIn(64, i-128);
        std::vector<int> dataOut = lut.compute(dataIn);
        std::cout << "Input data: " << i << "  ->->->  Output data: " << dataOut[0] << std::endl;
    }
    //float dataIn[4] = {1.0, 2.0, 150.0, 200.0};
    //int* data;
    //std::cout << "here!" << std::endl;
    //data = lut.compute(dataIn);
    //std::cout << "Result: " << data[0] << "  " << data[1] << std::endl;
 
    return 0;
}
