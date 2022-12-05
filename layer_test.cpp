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
    size_t bufferSize = 19;
    size_t depth = 3;
    size_t sizeFM = 5;
    size_t sizeK = 3;
    size_t stride = 2;

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

    Layer layer_test(bufferSize, depth, sizeFM, sizeK, stride, numK, devicePrecision, arraySizeX, arraySizeY, numADC, weight, lutNum, af);

    return 0;
}
