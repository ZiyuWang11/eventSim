// layer_test.cpp - test code for layer

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "buffer.h"
#include "tile.h"
#include "lut.h"
#include "layer.h"

const int dataPrecision = 8;
const int busWidth = 64;
const int memLatency = 20;

int main() {
    // Buffer configuration
    size_t bufferSize = 30;
    size_t depth = 3;
    size_t sizeFM = 10;
    size_t sizeK = 3;
    size_t stride = 1;

    // Tile configuration
    size_t numK = 16;
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 4;
    Eigen::MatrixXf weight = Eigen::MatrixXf::Ones(27, 16);
   
    // LUT configuration
    int lutNum = 4;
    std::string af = "ReLU";

    // Layer two configuration
    size_t sizeFM2 = 8;
    size_t depth2 = 16;
    size_t numK2 = 32;
    Eigen::MatrixXf weight2 = Eigen::MatrixXf::Ones(16*3*3, numK2);

    // Layer layer_test(bufferSize, depth, sizeFM, sizeK, stride, numK, devicePrecision, arraySizeX, arraySizeY, numADC, weight, lutNum, af);
    const int layerNum = 2;
    Layer layer_test[layerNum] = {
        Layer(bufferSize, depth, sizeFM, sizeK, stride, numK, devicePrecision, arraySizeX, arraySizeY, numADC, weight, lutNum, af),
        Layer(bufferSize, depth2, sizeFM2, sizeK, stride, numK2, devicePrecision, arraySizeX, arraySizeY, numADC, weight2, lutNum, af)
    };

    // Initialize clock
    long long int clock = 1;
    
    while (clock < 10000) {
        // Request Data from back to forward
        if (layer_test[layerNum].getRequest()) layer_test[layerNum].setOutTime(clock);
        for (int i = layerNum; i > 0; --i) {
            // Check if data can be pass through two layers
            if (layer_test[i].sendRequest() && layer_test[i-1].getRequest()) {
                std::cout << "Send data from Layer " << i << " to Layer " << i+1 << " at clock " << clock << std::endl;
                // Set Input and Input Time
                // Time and Data are get from previous layer
                Layer* layerPtr = &layer_test[i-1];
                layer_test[i].setInput(layerPtr);
                layer_test[i].setInputTime(clock, layerPtr);
                layer_test[i-1].setOutTime(clock);
            }
        }

        // Check if load data from memory for the first layer
        if (layer_test[0].sendRequest() /*if there is data in the main memory*/) {
            std::cout << "Send data from Memory to Layer 1 at clock " << clock << std::endl;
            // Use random number as input at this moment
            int value = (int)clock;
            std::vector<int> data(depth, value);
            layer_test[0].setInputFirst(data);
            layer_test[0].setInputTime(clock);
        }

        // Process Data 
        for (int i = 0; i < layerNum; ++i) {
            // Buffer -> Tile
            if (layer_test[i].buffer2tile()) {
                std::cout << "Layer " << i+1 << " sends data from buffer to tile at clock " << clock << std::endl;
                layer_test[i].setBuffer2Tile(clock);
            }
           
            // VMM
            if (layer_test[i].rdy4comp()) {
                std::cout << "Layer " << i+1 << " executes VMM at clock " << clock << std::endl;
                layer_test[i].setComp(clock);
            }
 
        }

        // State update
        for (int i = 0; i < layerNum; ++i) {
            layer_test[i].changeState(clock);
        }

        // Debug Code
        // if (clock % 1000 == 0) layer_test[1].checkBuffer();

        ++clock;
    }

    return 0;
}
