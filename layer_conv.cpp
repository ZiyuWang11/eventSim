// layer_conv.cpp - Convolution Layer Methods

#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"
#include "layer_conv.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

LayerConv::LayerConv(int layerNum, std::string layerType, // Layer
                     int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, // Tile
                     size_t numOut, int lutNum, std::string af, // LUT
                     size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding) // Buffer
                     : LayerVMM(layerNum, layerType, 
                                devicePrecision, arraySizeX, arraySizeY, numADC, weight, 
                                numOut, lutNum, af),
                       buffer(bufferSize, bufferDepth, sizeFM, sizeK, stride, padding)
{
//    std::cout << "Conv Layer Constructor!\n";
}

// Input Request Methods
// Check if the layer is ready to request data from the previous layer
bool LayerConv::sendRequest() const
{
    return buffer.loadRdy();
}

// Set the input time for loading data to buffer
// This time has already been calculated in LUT or global variable for memory
void LayerConv::setInputTime(long long int clockTime, LayerABC* prevLayer)
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
void LayerConv::setInputFirst(std::vector<int> data)
{
    buffer.loadData(data);
}

void LayerConv::setInput(LayerABC* prevLayer)
{
    std::vector<int> data = prevLayer->outData();
    buffer.loadData(data);
}

// Buffer -> Tile Methods
// Check if data can be send from buffer to tile
bool LayerConv::buffer2tile() const
{
    return buffer.sendRdy() && tile.loadRdy();
}

// Send data from buffer to tile
// and set latency, schedule event
void LayerConv::setBuffer2Tile(long long int clockTime)
{
    // Buffer send output data to tile input register
    // std::vector<std::vector<int>> data = buffer.sendData();
    tile.loadData(buffer.sendData());
    // tile set the data loading latency
    tile.setInTime(clockTime, buffer.sendTime());
    // buffer set data sending latency for move pointer
    buffer.setOutTime(clockTime);
}

// State Change Methods
// Change states of buffer and tile by checking clock
void LayerConv::changeState(long long int clockTime)
{
    // Move the pointer in Buffer
    buffer.movePtr(clockTime);
 
    // change the tile states
    tile.changeState(clockTime);
}

// Hardware Configuration
void LayerConv::layerConfig() const
{
    printf("===Layer %d - Conv===\n", layerNum_);
    buffer.bufferConfig();
    tile.tileConfig();
    lut.lutConfig();
    printf("====================\n");
    printf("\n");
}
