// tile.cpp - methods for CIM tile

#include <cmath>
#include <iostream>
#include <string>
#include <Eigen/Dense>
#include "tile.h"

extern const int dataPrecision;
extern const int busWidth; 

// Default Constructor
Tile::Tile()
{
    std::cout << "Default Constructor for Tile.\n";
    inputState_ = notRdy;
    arrayState_ = done;
    // sizeK_ = 3;
    // numK_ = 3;
    // channelDepth_ = 3;
    devicePrecision_ = 4;
    arraySizeX_ = 128;
    arraySizeY_ = 128;
    numADC_ = 4;
    // weight_.resize(sizeK_ * sizeK_ * channelDepth_, numK_);
    weight_.resize(1, 1);
    weight_.setZero();
    // input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.resize(1);
    input_.setZero();
    // output_.resize(numK_);
    output_.resize(1);
    output_.setZero();
}

// Constructor
Tile::Tile(/*size_t sizeK, size_t numK, size_t channelDepth, */int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight)
{
    // std::cout << "Constructor for Tile.\n";

    inputState_ = notRdy;
    arrayState_ = done;

    // NN model configuration
    // sizeK_ = sizeK;
    // numK_ = numK;
    // channelDepth_ = channelDepth;

    devicePrecision_ = devicePrecision;
    arraySizeX_ = arraySizeX;
    arraySizeY_ = arraySizeY;
    numADC_ = numADC;

    // event data initialization
    inputEventTile_ = false;
    inputEventTime_ = -1;
    arrayEventTile_ = false;
    arrayEventTime_ = -1;
    outputEventTile_ = false;
    outputEventTime_ = -1;

    weight_ = weight;

    arrayNumX_ = (2 * weight_.cols() * dataPrecision - 1) / devicePrecision_ / arraySizeX_+ 1;
    arrayNumY_ = (weight_.rows() - 1) / arraySizeY_ + 1;

    // input_.resize(sizeK_ * sizeK_ * channelDepth_);
    input_.resize(weight_.rows());
    input_.setZero();
    // output_.resize(numK_);
    output_.resize(weight_.cols());
    output_.setZero();

    // set latency for a layer VMM
    // set more complex model with ADC config
    latencyVMM_ =  10 // array
                 + 10 * ((arraySizeX_ - 1) / numADC_ + 1) // ADC conversion
                 + 2 // other digital logic
                 + ceil(log2(arrayNumX_)) * concHTree_ // tile concacetenation
                 + ceil(log2(arrayNumY_)) * addHTree_ // tile partial sum
                 ;

    
}

void Tile::changeState(long long int clockTime)
{
    // Input loading done
    if (clockTime == inputEventTime_) {
        // if get sufficient input data, change inputState
        if ((long int)inputHold_.size() == weight_.rows()) {
            inputHold_.clear();
            inputState_ = isRdy;
        }
        inputEventTile_ = false; // loading data event terminated
        //std::cout << "Load data to input register at Clock=" << clockTime << std::endl;
    }

    // Computation done
    if (clockTime == arrayEventTime_) {
        inputState_ = notRdy;
        arrayState_ = compute_done;
        arrayEventTile_ = false; // array execution terminated
        //std::cout << "VMM computation done at Clock=" << clockTime << std::endl;
    }
  
    // Output Sent
    if (clockTime == outputEventTime_) {
        arrayState_ = done;
        outputEventTile_ = false; // output data terminated
        //std::cout << "Send result to next layer at Clock=" << clockTime << std::endl;
    }
}
    
// Data loading methods
bool Tile::loadRdy() const
{
    return (inputState_ == notRdy) && !inputEventTile_;
}

// Set input ready time
void Tile::setInTime(long long int clockTime, int latency)
{
    // Schedule an event for loading input data
    inputEventTime_ = clockTime + latency; // This latency from Buffer
}

// If compute_done, input buffer can load data
void Tile::loadData(std::vector<int> data)
{
    // Load input data
    //std::vector<std::vector<int>> recvData = data; // maybe unnecessary

    // reshape received data to 1D vector fashion
    //std::vector<float> flattenData;
    //for (auto const& v: recvData) {
    //    flattenData.insert(flattenData.end(), v.begin(), v.end());
    //}

    // Push back data to temperal data holder
    //
    inputHold_.insert(inputHold_.end(), data.begin(), data.end());
 
    // Write data into input register if data length is sufficient
    if ((long int)inputHold_.size() == weight_.rows()) {

        std::vector<float> data_f(inputHold_.begin(), inputHold_.end());
        // convertdata to Eigen Vector
        input_ = Eigen::Map<Eigen::VectorXf>(data_f.data(), data_f.size());
    }

    // set a loading event for the input register
    inputEventTile_ = true;
}
    
// MVM computation methods
bool Tile::compRdy() const
{
    return (inputState_ == isRdy) && (arrayState_ == done);
}

// Set computation time
void Tile::setCompTime(long long int clockTime)
{
    // Schedule an event for VMM computation
    arrayEventTime_ = clockTime + latencyVMM_;
}

// If done && dataRdy, start compute
void Tile::computeVMM()
{
    // change array state to compute
    arrayState_ = compute;

    // VMM using Eigen package
    output_ = input_.transpose() * weight_;

    // set a computing event for the array
    arrayEventTile_ = true;
}

// Output Methods
bool Tile::outputRdy() const
{
    return (arrayState_ == compute_done) && !outputEventTile_;
}

void Tile::setOutTime(long long int clockTime, int latencyOut)
{
    // Schedule an event for sending output data
    outputEventTime_ = clockTime + latencyOut; // This latency from LUT
}

// get output to LUT
std::vector<int> Tile::getOutput()
{
    // Convert Eigen data in output register to std::vector in bus    
    std::vector<int> output(output_.data(), output_.data() + output_.size());

    // set an output event for the output register
    outputEventTile_ = true;
    
    return output;
}


// Debug code
void Tile::visTest() const
{
    //printf("==========Tile Configuration==========\n");
    //printf("- Array Size[%d, %d]\n", (int)arraySizeX_, (int)arraySizeY_);
    //printf("- Array Numbers[%d, %d]\n", (int)arrayNumX_, (int)arrayNumY_);

    //printf("==========Data Configuration==========\n");
    //printf("- Input Register Data:\n");
    //std::cout << input_ << std::endl;
    //printf("- Weight Data:\n");
    //std::cout << weight_ << std::endl;
    //printf("- Output Register Data:\n");
    //std::cout << output_ << std::endl;

    printf("Latency is %ld", latencyVMM_);

    // Print input register state
    switch (inputState_) {
        case 0: printf("Input Register State: isRdy\n"); break;
        case 1: printf("Input Register State: notRdy\n"); break;
        default: printf("NONE\n"); break;
    }
 
    // Print array state
    switch (arrayState_) {
        case 0: printf("Array State: Compute\n"); break;
        case 1: printf("Array State: Compute Done\n"); break;
        case 2: printf("Array State: Done\n"); break;
        default: printf("NONE\n"); break;
    }
}

void Tile::tileConfig() const
{
    printf("--------------------\n");
    printf("Device Bit: %d\n", devicePrecision_);
    printf("ADC per Array: %ld\n", numADC_);
    printf("Array size: [%ld, %ld]\n", arraySizeX_, arraySizeY_);
    printf("Number of Arrays: [%ld, %ld]\n", arrayNumX_, arrayNumY_);
    printf("Latency for a VMM: %ld\n", latencyVMM_);
} 

// Destructor
Tile::~Tile()
{
    // std::cout << "Array Destructor\n";
}

