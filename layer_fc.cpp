// layer_fc.cpp - Fully Connected Layer Methods

#include <iostream>
#include <string>
#include <vector>
#include "layer_fc.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

LayerFC::LayerFC(int layerNum, std::string layerType, // Layer
                     int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, // Tile
                     size_t numOut, int lutNum, std::string af) // LUT
                     : LayerVMM(layerNum, layerType, 
                                devicePrecision, arraySizeX, arraySizeY, numADC, weight, 
                                numOut, lutNum, af)
{
    std::cout << "FC Layer Constructor!\n";
}

// Input Request Methods
// Check if the layer is ready to request data from the previous layer
bool LayerFC::sendRequest() const
{
    return tile.loadRdy();
}

// Set the input time for loading data to buffer
// This time has already been calculated in LUT or global variable for memory
void LayerFC::setInputTime(long long int clockTime, LayerABC* prevLayer)
{
    // NOTE: for FC layer, use tile.setInTime method
    // For the first layer, latency is memory bus latency
    if (prevLayer == NULL) {
        tile.setInTime(clockTime, memLatency);
    }
    // For other layers, depend on output number, on-chip bus width, etc.
    // This parameter is calculated and stored in LUT
    else {
        tile.setInTime(clockTime, prevLayer->outTime());
    }
}

// Set the Input data
void LayerFC::setInputFirst(std::vector<int> data)
{
    tile.loadData(data);
}

void LayerFC::setInput(LayerABC* prevLayer)
{
    std::vector<int> data = prevLayer->outData();
    tile.loadData(data);
}

// State Change Methods
void LayerFC::changeState(long long int clockTime)
{
    // change the tile states
    tile.changeState(clockTime);
}
