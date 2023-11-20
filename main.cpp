// main.cpp - test code for LeNet and AlexNet

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "layer_conv.h"
#include "layer_fc.h"

const int dataPrecision = 8;
const int busWidth = 128;
const int memLatency = 20;

// Test LeNet and AlexNet, no pooling layer at this moment
void testLeNet(bool debug = false);
void testAlexNet(bool debug = false);

int main() {
    // If debug is true, will print debug code
    bool debug = true;

    testLeNet(debug);
    //testAlexNet(debug);

    return 0;
}

// LeNet Test - 2 Conv + 3 FC
void testLeNet(bool debug) { 
    // Buffer Configuration for inifinite buffer
    size_t bufferSize = 1000;
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 4;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";
    
    // Layer 1 Configuration
    // NN Configuration
    size_t depth1 = 3;
    size_t sizeFM1 = 31;
    size_t sizeK1 = 5;
    size_t stride1 = 2;
    size_t padding1 = 0;
    size_t numK1 = 6;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 configuration
    size_t sizeFM2 = 14;
    size_t sizeK2 = 6;
    size_t depth2 = 6;
    size_t stride2 = 2;
    size_t padding2 = 0;
    size_t numK2 = 16;
    Eigen::MatrixXf weight2 = Eigen::MatrixXf::Ones(sizeK2*sizeK2*depth2, numK2);

    // Layer 3 FC configuration
    size_t outNum3 = 120;
    Eigen::MatrixXf weight3 = Eigen::MatrixXf::Ones(numK2*5*5, outNum3);

    // Layer 4 FC configuration
    size_t outNum4 = 84;
    Eigen::MatrixXf weight4 = Eigen::MatrixXf::Ones(outNum3, outNum4);

    // Layer 5 FC configuration
    size_t outNum5 = 10;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(outNum4, outNum5);

    // Initialize Layers
    const int layerNum = 5;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, sizeFM1*(sizeK1-1)+1+sizeK1/*bufferSize*/, depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerConv(2, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight2, numK2, lutNum, af, sizeFM2*(sizeK2-1)+1+sizeK2/*bufferSize*/, depth2, sizeFM2, sizeK2, stride2, padding2);
    layer_test[2] = new LayerFC(3, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight3, outNum3, lutNum, af);
    layer_test[3] = new LayerFC(4, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight4, outNum4, lutNum, af);
    layer_test[4] = new LayerFC(5, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, outNum5, lutNum, af);
   
    printf("====Architecuture Configuration====\n");
    for (int i = 0; i < layerNum; ++i) {
        layer_test[i]->layerConfig();
    }

    // Initialize clock
    long long int clock = 1;

    // Initialize data to be processed
    size_t inPixel = 0;
    size_t outCount = 0; 
    size_t inputNum = 5;
    
    while (outCount < inputNum) {
        // Request Data from back to forward
        if (layer_test[layerNum-1]->getRequest()) {
            layer_test[layerNum-1]->setOutTime(clock);
            std::vector<int> output = layer_test[layerNum-1]->outData();
            ++outCount;
        }

        for (int i = layerNum - 1; i > 0; --i) {
            // Check if data can be pass through two layers
            if (layer_test[i]->sendRequest() && layer_test[i-1]->getRequest()) {
                if (debug) std::cout << "Send data from Layer " << i << " to Layer " << i+1 << " at clock " << clock << std::endl;
                // Set Input and Input Time
                // Time and Data are get from previous layer
                layer_test[i]->setInput(layer_test[i-1]);
                layer_test[i]->setInputTime(clock, layer_test[i-1]);
                layer_test[i-1]->setOutTime(clock);
            }
        }

        // Check if load data from memory for the first layer
        if (layer_test[0]->sendRequest() && (inPixel < sizeFM1 * sizeFM1 * inputNum)) {
            if (debug) std::cout << "Send data from Memory to Layer 1 at clock " << clock << std::endl;
            // Use random number as input at this moment
            int value = (int)clock;
            std::vector<int> data(depth1, value);
            layer_test[0]->setInputFirst(data);
            layer_test[0]->setInputTime(clock);
            ++inPixel;
        }

        // Process Data 
        for (int i = 0; i < layerNum; ++i) {
            // Buffer -> Tile
            if (layer_test[i]->buffer2tile()) {
                if (debug) std::cout << "Layer " << i+1 << " sends data from buffer to tile at clock " << clock << std::endl;
                layer_test[i]->setBuffer2Tile(clock);
            }
           
            // VMM
            if (layer_test[i]->rdy4comp()) {
                if (debug) std::cout << "Layer " << i+1 << " executes VMM at clock " << clock << std::endl;
                layer_test[i]->setComp(clock);
            }
 
        }

        // State update
        for (int i = 0; i < layerNum; ++i) {
            layer_test[i]->changeState(clock);
        }

        ++clock;
    }
    
    printf("==========================\n");
    printf("Terminate at Clock %lld\n", clock);
    printf("==========================\n");

}


// AlexNet Test - 5 Conv + 3 FC
void testAlexNet(bool debug) { 
    // Buffer Configuration 
    size_t bufferSize = 10000;
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 64;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";
    
    // Layer 1 Configuration
    // NN Configuration
    size_t depth1 = 3;
    size_t sizeFM1 = 224;
    size_t sizeK1 = 12;
    size_t stride1 = 4;
    size_t padding1 = 0;
    size_t numK1 = 96;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 configuration
    size_t sizeFM2 = 54;
    size_t sizeK2 = 6;
    size_t depth2 = 96;
    size_t stride2 = 2;
    size_t padding2 = 0;
    size_t numK2 = 256;
    Eigen::MatrixXf weight2 = Eigen::MatrixXf::Ones(sizeK2*sizeK2*depth2, numK2);

    // Layer 3 configuration
    size_t sizeFM3 = 25;
    size_t sizeK3 = 3;
    size_t depth3 = 256;
    size_t stride3 = 1;
    size_t padding3 = 0;
    size_t numK3 = 384;
    Eigen::MatrixXf weight3 = Eigen::MatrixXf::Ones(sizeK3*sizeK3*depth3, numK3);

    // Layer 4 configuration
    size_t sizeFM4 = 23;
    size_t sizeK4 = 3;
    size_t depth4 = 384;
    size_t stride4 = 2;
    size_t padding4 = 0;
    size_t numK4 = 384;
    Eigen::MatrixXf weight4 = Eigen::MatrixXf::Ones(sizeK4*sizeK4*depth4, numK4);

    // Layer 5 configuration
    size_t sizeFM5 = 11;
    size_t sizeK5 = 3;
    size_t depth5 = 384;
    size_t stride5 = 2;
    size_t padding5 = 0;
    size_t numK5 = 256;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(sizeK5*sizeK5*depth5, numK5);

    // Layer 6 FC configuration
    size_t outNum6 = 4096;
    Eigen::MatrixXf weight6 = Eigen::MatrixXf::Ones(numK5*5*5, outNum6);

    // Layer 7 FC configuration
    size_t outNum7 = 4096;
    Eigen::MatrixXf weight7 = Eigen::MatrixXf::Ones(outNum6, outNum7);

    // Layer 8 FC configuration
    size_t outNum8 = 1000;
    Eigen::MatrixXf weight8 = Eigen::MatrixXf::Ones(outNum7, outNum8);

    // Initialize Layers
    const int layerNum = 8;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, sizeFM1*(sizeK1-1)+1+sizeK1/*bufferSize*/, depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerConv(2, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight2, numK2, lutNum, af, sizeFM2*(sizeK2-1)+1+sizeK2/*bufferSize*/, depth2, sizeFM2, sizeK2, stride2, padding2);
    layer_test[2] = new LayerConv(3, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight3, numK3, lutNum, af, sizeFM3*(sizeK3-1)+1+sizeK3/*bufferSize*/, depth3, sizeFM3, sizeK3, stride3, padding3);
    layer_test[3] = new LayerConv(4, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight4, numK4, lutNum, af, sizeFM4*(sizeK4-1)+1+sizeK4/*bufferSize*/, depth4, sizeFM4, sizeK4, stride4, padding4);
    layer_test[4] = new LayerConv(5, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, numK5, lutNum, af, sizeFM4*(sizeK4-1)+1+sizeK4/*bufferSize*/, depth5, sizeFM5, sizeK5, stride5, padding5);
    layer_test[5] = new LayerFC(6, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight6, outNum6, lutNum, af);
    layer_test[6] = new LayerFC(7, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight7, outNum7, lutNum, af);
    layer_test[7] = new LayerFC(8, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight8, outNum8, lutNum, af);
   
    printf("====Architecuture Configuration====\n");
    for (int i = 0; i < layerNum; ++i) {
        layer_test[i]->layerConfig();
    }

    // Initialize clock
    long long int clock = 1;

    // Initialize data to be processed
    size_t inPixel = 0;
    size_t outCount = 0; 
    size_t inputNum = 2;
    
    while (outCount < inputNum) {
        // Request Data from back to forward
        if (layer_test[layerNum-1]->getRequest()) {
            layer_test[layerNum-1]->setOutTime(clock);
            std::vector<int> output = layer_test[layerNum-1]->outData();
            ++outCount;
        }

        for (int i = layerNum - 1; i > 0; --i) {
            // Check if data can be pass through two layers
            if (layer_test[i]->sendRequest() && layer_test[i-1]->getRequest()) {
                if (debug) std::cout << "Send data from Layer " << i << " to Layer " << i+1 << " at clock " << clock << std::endl;
                // Set Input and Input Time
                // Time and Data are get from previous layer
                layer_test[i]->setInput(layer_test[i-1]);
                layer_test[i]->setInputTime(clock, layer_test[i-1]);
                layer_test[i-1]->setOutTime(clock);
            }
        }

        // Check if load data from memory for the first layer
        if (layer_test[0]->sendRequest() && (inPixel < sizeFM1 * sizeFM1 * inputNum)) {
            if (debug) std::cout << "Send data from Memory to Layer 1 at clock " << clock << std::endl;
            // Use random number as input at this moment
            int value = (int)clock;
            std::vector<int> data(depth1, value);
            layer_test[0]->setInputFirst(data);
            layer_test[0]->setInputTime(clock);
            ++inPixel;
        }

        // Process Data 
        for (int i = 0; i < layerNum; ++i) {
            // Buffer -> Tile
            if (layer_test[i]->buffer2tile()) {
                if (debug) std::cout << "Layer " << i+1 << " sends data from buffer to tile at clock " << clock << std::endl;
                layer_test[i]->setBuffer2Tile(clock);
            }
           
            // VMM
            if (layer_test[i]->rdy4comp()) {
                if (debug) std::cout << "Layer " << i+1 << " executes VMM at clock " << clock << std::endl;
                layer_test[i]->setComp(clock);
            }
 
        }

        // State update
        for (int i = 0; i < layerNum; ++i) {
            layer_test[i]->changeState(clock);
        }

        ++clock;
    }
    
    printf("==========================\n");
    printf("Terminate at Clock %lld\n", clock);
    printf("==========================\n");

}
