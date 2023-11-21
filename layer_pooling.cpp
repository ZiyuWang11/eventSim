// layer_poolng.cpp - pooling layer methods

#include <string>
#include <vector>
#include "buffer.h"
#include "pooling.h"
#include "layerabc.h"
#include "layer_pooling.h"
#include <iostream>

extern const int dataPrecision;
extern const int busWidth;

LayerPooling::LayerPooling(int layerNum, std::string layerType, // Layer
                            std::string poolingType, // Pooling
                            size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding) // Buffer
                            : LayerABC(layerNum, layerType), 
                            pooling(bufferDepth, sizeK, poolingType),
                            buffer(bufferSize, bufferDepth, sizeFM, sizeK, stride, padding)
{

}

bool LayerPooling::sendRequest() const
{
    return buffer.loadRdy();
}

void LayerPooling::setInputTime(long long int clockTime, LayerABC* prevLayer)
{
    buffer.setInTime(clockTime, prevLayer->outTime()); 
}

void LayerPooling::setInput(LayerABC* prevLayer)
{
    std::vector<int> data = prevLayer->outData();
    buffer.loadData(data);
}

bool LayerPooling::getRequest() const
{
    return buffer.sendRdy();
}

std::vector<int> LayerPooling::outData()
{
    std::vector<int> output;
    output = pooling.compute(buffer.sendData());

    return output;
}

int LayerPooling::outTime() const
{
    return buffer.sendTime();
}

void LayerPooling::setOutTime(long long int clockTime)
{
    buffer.setOutTime(clockTime);
}

void LayerPooling::changeState(long long int clockTime)
{
    buffer.movePtr(clockTime);
}