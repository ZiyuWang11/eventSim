// layer_test.cpp - test code for layer

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
//#include "buffer.h"
//#include "tile.h"
//#include "lut.h"
#include "layer.h"

//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>

const int dataPrecision = 8;
const int busWidth = 256;
const int memLatency = 20;

int main() {
    // Test opencv
	//std::string image_path = cv::samples::findFile("test.tif");
	//cv::Mat img = cv::imread(image_path, cv::IMREAD_COLOR);

    // Buffer configuration
    size_t bufferSize = 100;
    size_t depth = 3;

    // Tile configuration

    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 4;
   
    // LUT configuration
    int lutNum = 4;
    std::string af = "ReLU";

    // Layer 1 configuration
    size_t sizeFM = 31;
    size_t sizeK = 5;
    size_t stride = 2;
    size_t numK = 6;
    Eigen::MatrixXf weight = Eigen::MatrixXf::Ones(sizeK*sizeK*depth, numK);
    // Layer 2 configuration
    size_t sizeFM2 = 14;
    size_t sizeK2 = 5;
    size_t depth2 = 6;
    size_t stride2 = 1;
    size_t numK2 = 16;
    Eigen::MatrixXf weight2 = Eigen::MatrixXf::Ones(sizeK2*sizeK2*depth2, numK2);

    // For conv layer test
    size_t outNum = 6;
    size_t outNum2 = 16;

    printf("==Architecuture Configuration==\n");
    // Layer layer_test(bufferSize, depth, sizeFM, sizeK, stride, numK, devicePrecision, arraySizeX, arraySizeY, numADC, weight, lutNum, af);
    const int layerNum = 2;
    Layer layer_test[layerNum] = {
        Layer(1, "Conv", bufferSize-32, depth, sizeFM, sizeK, stride, numK, devicePrecision, arraySizeX, arraySizeY, numADC, weight, lutNum, af),
        Layer(2, "Conv", bufferSize-66, depth2, sizeFM2, sizeK2, stride2, numK2, devicePrecision, arraySizeX, arraySizeY, numADC, weight2, lutNum, af)
    };

    //LayerConv layer_test[layerNum] = {
    //    LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight, outNum, lutNum, af, bufferSize, depth, sizeFM, sizeK, stride),
    //    LayerConv(2, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight2, outNum2, lutNum, af, bufferSize, depth2, sizeFM2, sizeK2, stride2)
    //};

    // Initialize clock
    long long int clock = 1;

    // Initialize data to be processed
    const int outNumber = 10 * 10;
    int outCount  = 0; 
    
    while (outCount < outNumber) {
        // Request Data from back to forward
        if (layer_test[layerNum-1].getRequest()) {
            layer_test[layerNum-1].setOutTime(clock);
            std::vector<int> output = layer_test[layerNum-1].outData();
            ++outCount;
            // std::cout << outCount << std::endl;
        }

        for (int i = layerNum - 1; i > 0; --i) {
            // Check if data can be pass through two layers
            if (layer_test[i].sendRequest() && layer_test[i-1].getRequest()) {
                // std::cout << "Send data from Layer " << i << " to Layer " << i+1 << " at clock " << clock << std::endl;
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
            // std::cout << "Send data from Memory to Layer 1 at clock " << clock << std::endl;
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
                // std::cout << "Layer " << i+1 << " sends data from buffer to tile at clock " << clock << std::endl;
                layer_test[i].setBuffer2Tile(clock);
            }
           
            // VMM
            if (layer_test[i].rdy4comp()) {
                //std::cout << "Layer " << i+1 << " executes VMM at clock " << clock << std::endl;
                layer_test[i].setComp(clock);
            }
 
        }

        // State update
        for (int i = 0; i < layerNum; ++i) {
            layer_test[i].changeState(clock);
        }

        // Debug Code
        //if (clock % 10000 == 0) {
        //    std::cout << "Check at clock: " << clock << std::endl;
        //    //layer_test[1].checkBuffer();
        //    layer_test[1].checkTile();
        //}
        ++clock;
    }

    printf("==========================\n");
    printf("Terminate at Clock %lld\n", clock);
    printf("==========================\n");

    return 0;
}
