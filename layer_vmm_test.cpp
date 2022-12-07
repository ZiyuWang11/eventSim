// layer_vmm_test.cpp - Test LayerVMM, LayerConv and LayerFC

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "layer_conv.h"

const int dataPrecision = 8;
const int busWidth = 64;
const int memLatency = 20;

int main() {
    // Matrix size
    size_t M = 27; 
    size_t N = 16;

    // Tile configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 4;
    Eigen::MatrixXf weight = Eigen::MatrixXf::Ones(M, N);
   
    // LUT configuration
    int lutNum = 4;
    std::string af = "ReLU";

    // Buffer configuration
    size_t bufferSize = 30;
    size_t depth = 3;
    size_t sizeFM = 10;
    size_t sizeK = 3;
    size_t stride = 1;    

    // LayerVMM object
    // LayerVMM layerVMM_test(1, "Test", devicePrecision, arraySizeX, arraySizeY, numADC, weight, N, lutNum, af);

    // Layer Conv object
    LayerConv layerConv_test(1, "Test", devicePrecision, arraySizeX, arraySizeY, numADC, weight, N, lutNum, af, bufferSize, depth, sizeFM, sizeK, stride);

}
