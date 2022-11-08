// buffer.cpp - buffer methods
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"


// Default constructor
Buffer::Buffer()
{
    std::cout << "Default constructor" << std::endl;
    bufferSize_ = 0;
    bufferDepth_ = 0;
    headPtr_ = 0;
    tailPtr_ = 0;
    bufferData.resize(bufferSize_, std::vector<uint8_t>(bufferDepth, 0));
}


// Constructor
Buffer::Buffer(size_t bufferSize, size_t bufferDepth)
{
    std::cout << "Constructor" << std::endl;
    bufferSize_ = bufferSize;
    bufferDepth_ = bufferDepth;
    headPtr_ = 0;
    tailPtr_ = 0;
    bufferData.resize(bufferSize_, std::vector<uint8_t>(bufferDepth, 0));
}

// check buffer status
bool Buffer::isFull() const
{
    return tailPtr_ == (headPtr_ + 1) % bufferSize_;
}

// check whether can send data to array
// 1. heatPtr is above tailPtr a conv window
// 2. array is at wait state
// change this when friend class array is ready
bool Buffer::sendRdy(/*Array& array*/ bool arrayRdy) const 
{
    if (/*array.getState()*/
        arrayRdy && dataNum_ >= 2 * sizeFM_ + sizeK_) {
        return true;
    }
}

// send data to array - constrained by bus width
// each data is 8 or 4 bit
std::vector<std::vector<uint8_t>> Buffer::sendData()
{
    eventWrapper(timeGlobal, "Buffer send data");
    int window = sizeK_ * sizeK_
    vector<std::vector<uint8_t>> dataCovWindow;
    // calculate clock cycles to send all data fow a conv window
    int tSend = window * bufferDepth_ * datePrecision / busWidth;
    timeGlobal += tSend;

    for (auto i = 0; i < sizeK_; ++i) {
        for (auto j = 0; j < sizeK_; ++j) {
            int idx = (headPtr_ + j + i * sizeFM_) % bufferSize_;
            dataConvWindow.push_back(bufferData_[idx]);
        }
    }

    ++tailPtr_;
    --dataNum_;

    return dataConvWindow;
}

// check whether can load data
// check previous layer implemented in high level Layer class
bool Buffer::loadRdy() const
{
    return !isFull();
}

// load data to one block of the buffer
void Buffer::loadData(std::vector<uint8_t> data)
{
    evnetWrapper(timeGlobal, "Buffer load data");
    bufferData[++headPtr] = data;
    ++dataNum_;
    int tLoad = bufferDepth_ * dataPrecision / busWidth;
    timeGlobal += tLoad;
}

// to be changed later
void Buffer::eventWrapper(int eventTime, string& event) const
{
    std::cout << "Time " << eventTime << " : " << event << std::endl;
}

// Debug code for test only
void Buffer::visTest() const
{
    for (auto i = 0; i < bufferSize_; ++i) {
        std::cout << i << ": ";
        for (auto j = 0; j < bufferDepth_; ++j) {
            std::cout << bufferData_[i][j] << " ";
        }
        if (i == headPtr_) {
            std::cout << "Head" << std::endl;
        } 
        else if (i == tailPtr_) {
            std::cout << "Tail" << std::endl;
        }
        else {
            std::cout << " " << std::endl;
        }
    }
}

// Destructor
Buffer::~Buffer()
{
    std::cout << "Destructor" << std::endl;
}
