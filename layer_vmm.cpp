// layer_vmm.cpp - methods for VMM layers

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "tile.h"
#include "lut.h"
#include "layer_vmm.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

LayerVMM::LayerVMM(int layerNum, std::string layerType, 
                   int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, 
                   size_t numOut, int lutNum, std::string af) 
                   : LayerABC(layerNum, layerType), 
                     tile(devicePrecision, arraySizeX, arraySizeY, numADC, weight), 
                     lut(numOut, lutNum, af)
{
    std::cout << "Layer Constructor\n";
    //printf("--------------------\n");
    //printf("Input Feature Map: %ld\n", sizeFM);
    //printf("Flatten Matrix Size: [%ld, %ld]\n", sizeK*sizeK*depth, numK);
    //std::cout << "Activation function: " << af << std::endl;
    //printf("====================\n");
    //std::cout << "Layer " << layerNum_ << " - " << layerType_ << std::endl;
    //printf("====================\n\n");
}

// Output response methods
bool LayerVMM::getRequest() const
{
    return tile.outputRdy();
}

// Send output data from tile to lut
std::vector<int> LayerVMM::outData()
{
    std::vector<int> output;
    output = lut.compute(tile.getOutput());

    return output;
}

// Get the output latency
int LayerVMM::outTime() const
{
    return lut.getTime();
}

// Schedule an output event change in tile
void LayerVMM::setOutTime(long long int clockTime)
{
    tile.setOutTime(clockTime, lut.getTime());
}

// Check if tile is ready for computation
bool LayerVMM::rdy4comp() const
{
    return tile.compRdy();
}

// Run VMM and schedule the event for computation
void LayerVMM::setComp(long long int clockTime)
{
    tile.computeVMM();
    tile.setCompTime(clockTime);
}

