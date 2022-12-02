// buffer_test.cpp - test buffer class
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"

const int busWidth = 64;
const int dataPrecision = 8;

int main()
{
    long long int clock = 1;
    int inLatency = 9;

    size_t bufferSize = 19;
    size_t bufferDepth = 2;
    size_t sizeFM = 5;
    size_t sizeK = 3;
    size_t stride = 2;

    Buffer test_buffer(bufferSize, bufferDepth, sizeFM, sizeK, stride);

    while (clock < 250) {
        // Try to write new data
        int value = (int)clock;
        if (test_buffer.loadRdy()) {
            std::vector<int> data(bufferDepth, value);
            test_buffer.loadData(data);
            test_buffer.setTime(clock, inLatency);
        }

        // Try to send data from buffer
        if (test_buffer.sendRdy()) {
            int time = test_buffer.sendTime();
            std::vector<std::vector<int>> outdata = test_buffer.sendData();
            std::cout << "Schedule sending data at " << clock + time << std::endl;
        }
        // State updata
        test_buffer.movePtr(clock);
        
        // Visualize buffer
        if (clock%10 == 0) test_buffer.visTest();

        ++clock;
    }

    //test_buffer.visTest();
    //size_t value = 1;
    //while (test_buffer.loadRdy()) {
    //    std::vector<uint8_t> data(bufferDepth, value);
    //    test_buffer.loadData(data);
    //    ++value;
        
        //std::cout << "Buffer at time: " << timeGlobal << std::endl;
        //test_buffer.visTest();
    //}

    //for (int i = 0; i < 5; ++i) {
    //    test_buffer.visTest();
    //    std::vector<uint8_t> data(bufferDepth, value + 20);
    //    std::vector<std::vector<uint8_t>> outData = test_buffer.sendData();
    //    for (size_t k = 0; k < sizeK * sizeK; ++k) {
    //        std::cout << (unsigned)outData[k][0] << " ";
    //        if (k % 3 == 2) std::cout << "\n";
    //    }
    //    std::cout<< "\n";
    //    test_buffer.visTest();
    //    test_buffer.loadData(data);
    //    test_buffer.visTest();
    //} 

    return 0;
}
