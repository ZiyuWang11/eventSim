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
extern const int memLatency;

// Constructor
// Using sub-module Constructors
Layer::Layer(size_t bufferSize, size_t depth, size_t sizeFM, size_t sizeK, size_t stride, size_t numK, int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, int lutNum, std::string af) : buffer(bufferSize, depth, sizeFM, sizeK, stride), tile(sizeK, numK, depth, devicePrecision, arraySizeX, arraySizeY, numADC, weight), lut(numK, lutNum, af)
{
    std::cout << "Layer Constructor\n";
}

// Check if the layer is ready to request data from the previous layer
// NOTE: FC layer is different, implement it later
bool Layer::sendRequest() const
{
    return buffer.loadRdy();
}

// Check if previous layer accept the request
// For the first layer, always accept the request, and the latency is the memory latency
bool Layer::getResponse(Layer* prevLayer) const
{
    // For first layer, no previous layer
    // may need some flags here later
    if (prevLayer == NULL) {
        return true;
    }
    // Other layers
    else {
        return prevLayer->getRequest();
    }
}

// Set the input time for loading data to buffer
// This time has already been calculated in LUT or global variable for memory
void Layer::setInputTime(long long int clockTime, Layer* prevLayer)
{
    // NOTE: for FC layer, use tile.setInTime method
    // For the first layer, latency is memory bus latency
    if (prevLayer == NULL) {
        buffer.setInTime(clockTime, memLatency);
    }
    // For other layers, depend on output number, on-chip bus width, etc.
    // This parameter is calculated and stored in LUT
    else {
        buffer.setInTime(clockTime, prevLayer->outTime());
    }
}

// Set the Input data
void Layer::setInput(Layer* prevLayer)
{
    // Add another paramter to this method for first layer
    // Maybe use cv::Mat to store input data
    if (prevLayer == NULL) {

    }
    else {
        buffer.loadData(prevLayer->outData());
    }
}

// Check if ready to get request/send output
// Check tile outputRdy() method
bool Layer::getRequest() const
{
    return tile.outputRdy();
}

// Send output data from tile to lut
std::vector<int> Layer::outData()
{
    std::vector<int> output;;
    output = lut.compute(tile.getOutput());

    return output;
}

// Get the output latency
int Layer::outTime() const
{
    // NOTE: pooling layer has no lut, think it later
    return lut.getTime();
}

// Schedule an output event change in tile
void Layer::setOutTime(long long int clockTime)
{
    tile.setOutTime(clockTime, lut.getTime());
}

// Check if data can be send from buffer to tile
bool Layer::buffer2tile() const
{
    return buffer.sendRdy() && tile.loadRdy();
}

// Send data from buffer to tile
// and set latency, schedule event
void Layer::setBuffer2Tile(long long int clockTime)
{
    // Buffer send output data to tile input register
    std::vector<std::vector<int>> data = buffer.sendData();
    tile.loadData(data);
    // tile set the data loading latency
    tile.setInTime(clockTime, buffer.sendTime());
    // buffer set data sending latency for move pointer
    buffer.setOutTime(clockTime);
}

// Check if tile is ready for computation
bool Layer::rdy4comp() const
{
    return tile.compRdy();
}

// Run VMM and schedule the event for computation
void Layer::setComp(long long int clockTime)
{
    tile.computeVMM();
    tile.setCompTime(clockTime);
}

// Change states of buffer and tile by checking clock
void Layer::changeState(long long int clockTime)
{
    // Move the pointer in Buffer
    buffer.movePtr(clockTime);
 
    // change the tile states
    tile.changeState(clockTime);
}

// Destructor
Layer::~Layer()
{
    std::cout << "Layer Destructor\n";
}
