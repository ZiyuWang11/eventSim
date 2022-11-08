// buffer_test.cpp - test buffer class
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"

int timeGlobal = 0;
const int busWidth = 64;
const int dataPrecision = 8;

int main()
{
    size_t bufferSize = 19;
    size_t bufferDepth = 2;
    size_t sizeFM = 8;
    size_t sizeK = 3;

    Buffer test_buffer(bufferSize, bufferDepth, sizeFM, sizeK);

    //test_buffer.visTest();
    size_t value = 1;
    while (test_buffer.loadRdy()) {
        std::vector<uint8_t> data(bufferDepth, value);
        test_buffer.loadData(data);
        ++value;
        
        //std::cout << "Buffer at time: " << timeGlobal << std::endl;
        //test_buffer.visTest();
    }

    for (int i = 0; i < 5; ++i) {
        test_buffer.visTest();
        std::vector<uint8_t> data(bufferDepth, value + 20);
        std::vector<std::vector<uint8_t>> outData = test_buffer.sendData();
        for (size_t k = 0; k < sizeK * sizeK; ++k) {
            std::cout << (unsigned)outData[k][0] << " ";
            if (k % 3 == 2) std::cout << "\n";
        }
        std::cout<< "\n";
        std::cout << "Buffer at time: " << timeGlobal << std::endl;
        test_buffer.visTest();
        test_buffer.loadData(data);
        test_buffer.visTest();
    } 

    return 0;
}
