// layer.cpp - layer methods

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "buffer.h"
#include "tile.h"
#include "lut.h"
#include "layer.h"

extern const int dataPrecision;
extern const int busWidth;

// Constructor
// Using sub-module Constructors
Layer::Layer(size_t bufferSize, size_t depth, size_t sizeFM, size_t sizeK, size_t stride, size_t numK, int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, int lutNum, std::string af) : buffer(bufferSize, depth, sizeFM, sizeK, stride), tile(sizeK, numK, depth, devicePrecision, arraySizeX, arraySizeY, numADC, weight), lut(numK, lutNum, af)
{
    std::cout << "Layer Constructor\n";
}

// Destructor
Layer::~Layer()
{
    std::cout << "Layer Destructor\n";
}
