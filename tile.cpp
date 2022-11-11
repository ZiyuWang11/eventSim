// tile.cpp - methods for CIM tile

#include <cmath>
#include <iostream>
#include <string>
#include <Eigen/Dense>
#include "tile.h"

extern int timeGlobal;
extern const int dataPrecision;
extern const int busWidth; 

// Default Constructor
Tile::Tile()
{
    std::cout << "Default Constructor for Tile.\n";
    inputState_ = notRdy;
    arrayState_ = done;
    sizeK_ = 3;
    numK_ = 3;
    channelDepth_ = 3;
    devicePrecision_ = 4;
    arraySizeX_ = 128;
    arraySizeY_ = 128;
    numADC_ = 4;
    weight_.resize(sizeK_ * sizeK_ * channelDepth_, numK_);
    weight_.setZero();
    input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.setZero();
    output_.resize(numK_);
    output_.setZero();
}

// Constructor
Tile::Tile(size_t sizeK, size_t numK, size_t channelDepth, int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight)
{
    std::cout << "Constructor for Tile.\n";

    inputState_ = notRdy;
    arrayState_ = done;

    // NN model configuration
    sizeK_ = sizeK;
    numK_ = numK;
    channelDepth_ = channelDepth;

    devicePrecision_ = devicePrecision;
    arraySizeX_ = arraySizeX;
    arraySizeY_ = arraySizeY;
    numADC_ = numADC;

    weight_ = weight;

    arrayNumX_ = (2 * weight_.cols() * dataPrecision - 1) / devicePrecision_ / arraySizeX_+ 1;
    arrayNumY_ = (weight_.rows() - 1) / arraySizeY_ + 1;

    input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.setZero();
    output_.resize(numK_);
    output_.setZero();

    latencyMVM_ = 10;
}

// set latency for a layer MVM
// set more complex model with ADC config
void Tile::setLatency()
{
    latencyMVM_ =  10 // array
                 + 10 * ((arraySizeX_ - 1) / numADC_ + 1) // ADC conversion
                 + 2 // other digital logic
                 + (int)(log2(arrayNumX_ - 1) + 1) * concHTree_ // tile concacetenation
                 + (int)(log2(arrayNumY_ - 1) + 1) * addHTree_ // tile partial sum
                 ;
}
    
// Dataloader
// If compute_done, input buffer can load data
void Tile::loadData(std::vector<std::vector<uint8_t>> data)
{
    if (inputRdy()) {
        // event - load done
        // event time handled in buffer, only report load done here
        std::string eventName = "Loaded Data to Input Register";
        eventWrapper(timeGlobal, eventName);
    
        // Load input data
        // std::vector<vector<uint8_t>> recvData = buffer.sendData();
        std::vector<std::vector<uint8_t>> recvData = data;
        // reshape received data to 1D vector fashion
        std::vector<float> flattenData;
        for (auto const& v: recvData) {
            flattenData.insert(flattenData.end(), v.begin(), v.end());
        }
        // convertdata to Eigen Vector
        input_ = Eigen::Map<Eigen::VectorXf>(flattenData.data(), flattenData.size());

        inputState_ = isRdy;
    } else {
        std::string eventName = "Hold dataloading";
        eventWrapper(timeGlobal, eventName);
    }
}
    
// MVM computation
// If done && dataRdy, start compute
void Tile::computeMVM()
{
    if (compRdy()) {
        std::string eventName = "VMM computation";
        eventWrapper(timeGlobal, eventName);
        timeGlobal += latencyMVM_;
    
        output_ = input_.transpose() * weight_;

        arrayState_ = compute_done;
    } else {
        std::string eventName = "Hold MVM computation";
        eventWrapper(timeGlobal, eventName);
    }
}

// get output to LUT
std::vector<uint8_t> Tile::getOutput()
{
    // depend on LUT number
    // implement with more detail when LUT done
    // label popped registers

    //if (outputRdy()) {
        std::string eventName = "send VMM output to LUT";
        eventWrapper(timeGlobal, eventName);
        
        arrayState_ = done;
        
        std::vector<uint8_t> output(output_.data(), output_.data() + output_.size());
        return output;
    /*} else {
        std::string eventName = "Hold VMM results";
        break;
    }*/
}


// Debug code
void Tile::visTest() const
{
    printf("==========Tile Configuration==========\n");
    printf("- Array Size[%d, %d]\n", (int)arraySizeX_, (int)arraySizeY_);
    printf("- Array Numbers[%d, %d]\n", (int)arrayNumX_, (int)arrayNumY_);

    printf("==========Data Configuration==========\n");
    printf("- Input Register Data:\n");
    std::cout << input_ << std::endl;
    printf("- Weight Data:\n");
    std::cout << weight_ << std::endl;
    printf("- Output Register Data:\n");
    std::cout << output_ << std::endl;
}
 
// Record event - load/send data, buffer full
// Simple print now, update it when event table is ready
void Tile::eventWrapper(int eventTime, std::string& event) const
{
    std::cout << "Time " << eventTime << " : " << event << std::endl;
}

// Destructor
Tile::~Tile()
{
    std::cout << "Array Destructor\n";
}

