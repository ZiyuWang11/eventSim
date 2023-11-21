// pooling.cpp - pooling unit methods

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>
#include <string>
#include "pooling.h"

extern const int dataPrecision;
extern const int busWidth;

// Default constructor
Pooling::Pooling()
{
    std::cout << "Default Constructor" << std::endl;
}

// Constructor
Pooling::Pooling(size_t channelDepth, size_t filterSize, std::string poolingType) : poolingType_(poolingType), channelDepth_(channelDepth), filterSize_(filterSize)
{
    outLatency_ = (int) ((channelDepth_ * dataPrecision- 1) / busWidth + 1);
}

int Pooling::getTime() const
{
    return outLatency_;
}

std::vector<int> Pooling::compute(std::vector<int> dataIn)
{
    std::vector<std::vector<int>> dataInReshape;
    for (auto i = dataIn.begin(); i < dataIn.end(); i += filterSize_ * filterSize_) {
        dataInReshape.push_back({i, i + filterSize_ * filterSize_});
    }

    std::vector<int> dataOut;
    for (int i = 0; i < dataInReshape.size(); ++i) {
        if (poolingType_ == "Max") {
            dataOut.push_back(*std::max_element(dataInReshape[i].begin(), dataInReshape[i].end()));
        }
        else if (poolingType_ == "Ave") {
            dataOut.push_back(std::accumulate(dataInReshape[i].begin(), dataInReshape[i].end(), 0.0) / dataInReshape[i].size());
        }
    }

    return dataOut;
}

// Destructor
Pooling::~Pooling()
{

}