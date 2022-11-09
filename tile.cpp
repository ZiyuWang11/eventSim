// tile.cpp - methods for CIM tile

#include <cmath>
#include <iostream>
#include <string>
#include <Eigen/Dense>

extern int timeGlobal;
extern const int dataPrecision;
extern const int busWidth; 

// Default Constructor
Tile::Tile()
{
    std::cout << "Default Constructor for Tile.\n";
    state_ = done;
    sizeK_ = 3;
    numK_ = 3;
    channelDepth_ = 3;
    devicePrecision_ = 4;
    arraySizeX_ = 128;
    arraySizeY_ = 128;
    numADC = 4;
    weight_.resize(sizeK_ * sizeK_ * channelDepth_, numK_);
    weight_.setZero();
    input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.setZero();
    output_resize(numK_);
    output_.setZero();
}

// Constructor
Tile::Tile(int state = done, size_t sizeK, size_t numK, size_t channelDepth, int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const MatrixXf& weight)
{
    std::cout << "Constructor for Tile.\n"

    state_ = state;

    // NN model configuration
    sizeK_ = sizeK;
    numK_ = numK;
    channelDepth_ = channelDepth;

    devicePrecision_ = devicePrecision;
    arraySizeX_ = arraySizeX;
    arraySizeY_ = arraySizeY;
    numADC_ = numADC;

    weight_ = weight;

    arrayNumX_ = (2 * weight_.col() * dataPrecision - 1) / devicePrecision_ / arraySizeX_+ 1;
    arrayNumY_ = (weight_.row() - 1) / arraySizeY_ + 1;

    input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.setZero();
    output_resize(numK_);
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
void loadData(Buffer& buffer)
{
    // convert data to Eigen
    // reshape?
    // write to input buffer
    // event - load done
}
    
// MVM computation
void computeMVM()
{
    // simple MVM using eigen
    // record event
    // result write to out reg
}

// get output to LUT
VectorXf getOutput()
{
    // depend on LUT number
    // implement with more detail when LUT done
    // label popped registers
}
 
// Record event - load/send data, buffer full
// Simple print now, update it when event table is ready
void eventWrapper(int eventTime, std::string& event) const
{
    std::cout << "Time " << eventTime << " : " << event << std::endl;
}

// Destructor
Tile::~Tile()
{
    std::cout << "Array Destructor\n";
}

