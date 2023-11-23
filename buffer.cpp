// buffer.cpp - buffer methods
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"

extern const int dataPrecision;
extern const int busWidth; 

// Default constructor
Buffer::Buffer()
{
    std::cout << "Default constructor" << std::endl;
    headEventBuffer_ = false;
    headEventTime_ = -1;
    tailEventBuffer_ = false;
    tailEventTime_ = -1;
    bufferSize_ = 0;
    bufferDepth_ = 0;
    headPtr_ = 0;
    tailPtr_ = 0;
    sizeFM_ = 28;
    sizeK_ = 3;
    stride_ = 1;
    padding_ = 0;
    dataNum_ = 0;
    bufferData_.resize(bufferSize_, std::vector<int>(bufferDepth_, 0));
}


// Constructor
Buffer::Buffer(size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding, bool singlePadding)
{
    // std::cout << "Constructor" << std::endl;
    headEventBuffer_ = false;
    headEventTime_ = -1;
    tailEventBuffer_ = false;
    tailEventTime_ = -1;
    bufferSize_ = bufferSize;
    bufferDepth_ = bufferDepth;
    sizeFM_ = sizeFM;
    sizeK_ = sizeK;
    stride_ = stride;
    padding_ = padding;
    padding_ = padding;
    singlePadding_ = singlePadding;
    step_ = 0; // step_ < (sizeFM_ - sizeK_) / stride_ + 1;
    stepCol_ = 0;
    inputFMCnt_ = 0;
    inputFMCnt_ = 0;
    headPtr_ = 0;
    tailPtr_ = 0;
    if (!singlePadding_) {
        dataNum_ = padding_ * (sizeFM_ + 2 * padding_) + padding_;
        sizeFMwPadding_ = sizeFM_ + 2 * padding_;
        sizeOFM_ = (sizeFM_ - sizeK_ + 2 * padding_) / stride_ + 1;
    }
    else {
        dataNum_ = padding_ * (sizeFM_ + padding_);
        sizeFMwPadding_ = sizeFM_ + padding_;
        sizeOFM_ = (sizeFM_ - sizeK_ + padding_) / stride_ + 1;
    }
    headPtr_ = (headPtr_ + dataNum_) % bufferSize_;
    bufferData_.resize(bufferSize_, std::vector<int>(bufferDepth, 0));
    buffer2tileLatency_ = sizeK_ * sizeK_ * bufferDepth_ * dataPrecision / busWidth;

}

// check if ready to load new data
bool Buffer::loadRdy() const
{
    if ((inputFMCnt_ + 1) % (sizeFM_ * sizeFM_) == 0) {
        // std::cout << "Corner" << std::endl;
        int distance = padding_ + padding_ * sizeFMwPadding_ + padding_ * sizeFMwPadding_ + padding_;
        bool loadAvailable = true;
        for (int i = 0; i <= distance; ++i) {
            loadAvailable &= (tailPtr_ != (headPtr_ + 1 + distance) % bufferSize_);
        }
        return loadAvailable && !headEventBuffer_;
    }
    else if ((inputFMCnt_ + 1) % sizeFM_ == 0) {
        // std::cout << "Edge" << std::endl;
        int distance = 2 * padding_;
        bool loadAvailable = true;
        for (int i = 0; i <= distance; ++i) {
            loadAvailable &= (tailPtr_ != (headPtr_ + 1 + distance) % bufferSize_);
        }
        return loadAvailable && !headEventBuffer_;
    }
    else {
        return (tailPtr_ != (headPtr_ + 1) % bufferSize_) && !headEventBuffer_;
    }
}

// shedule ready time for the input data
void Buffer::setInTime(long long int clockTime, int latency)
{
    // schedule the time for headPtr
    headEventTime_ = clockTime + latency;
}


// load data to one block of the buffer
void Buffer::loadData(std::vector<int> data)
{

    bufferData_[headPtr_] = data;

    headEventBuffer_ = true;
}

// move headPtr for new data after loading latency
void Buffer::movePtr(long long int clockTime)
{
    if (clockTime == headEventTime_) {
        headPtr_ = (headPtr_ + 1) % bufferSize_;
        ++dataNum_;
        inputFMCnt_ = (inputFMCnt_ + 1) % (sizeFM_ * sizeFM_);
        headEventBuffer_ = false; // event terminate

        if (inputFMCnt_ % (sizeFM_ * sizeFM_) == 0) {
            if (singlePadding_) {
                for (int i = 0; i < padding_; ++i) {
                    int idx = (headPtr_ + i) % bufferSize_;
                    bufferData_[idx] = std::vector<int>(bufferDepth_, 0);
                }
                
                headPtr_ = (headPtr_ + padding_ + padding_ * sizeFMwPadding_) % bufferSize_;
                dataNum_ += padding_ + padding_ * sizeFMwPadding_;
            }
            else {
                for (int i = 0; i < padding_ + padding_ * sizeFMwPadding_; ++i) {
                    int idx = (headPtr_ + i) % bufferSize_;
                    bufferData_[idx] = std::vector<int>(bufferDepth_, 0);
                }
                
                headPtr_ = (headPtr_ + padding_ + padding_ * sizeFMwPadding_ + padding_ * sizeFMwPadding_ + padding_) % bufferSize_;
                dataNum_ += padding_ + padding_ * sizeFMwPadding_ + padding_ * sizeFMwPadding_ + padding_;
            }
        }
        // Load a row of input FM
        else if (inputFMCnt_ % sizeFM_ == 0) {
            if (singlePadding_) {
                for (int i = 0; i < padding_; ++i) {
                    int idx = (headPtr_ + i) % bufferSize_;
                    bufferData_[idx] = std::vector<int>(bufferDepth_, 0);
                }

                headPtr_ = (headPtr_ + padding_) % bufferSize_;
                dataNum_ += padding_;
            }
            else {
                for (int i = 0; i < 2 * padding_; ++i) {
                    int idx = (headPtr_ + i) % bufferSize_;
                    bufferData_[idx] = std::vector<int>(bufferDepth_, 0);
                }

                headPtr_ = (headPtr_ + 2 * padding_) % bufferSize_;
                dataNum_ += 2 * padding_;
            }
        }
    }

    // send one conv window
    if (clockTime == tailEventTime_) {
        if (stepCol_ < sizeOFM_) {
            if (step_ < sizeOFM_) {
                tailPtr_ = (tailPtr_ + stride_) % bufferSize_;
                dataNum_ -= stride_;
                ++step_;
            }
            else {
                
                tailPtr_ = (tailPtr_ + sizeK_ + (stride_ - 1) * sizeFMwPadding_) % bufferSize_;
                dataNum_ -= sizeK_ + (stride_ - 1) * sizeFMwPadding_;
                step_ = 0;
                ++stepCol_;
            }
        }
        else {
            tailPtr_ = (tailPtr_ + sizeK_ + (sizeK_ - 1) * sizeFMwPadding_) % bufferSize_;
            dataNum_ -= sizeK_ + (sizeK_ - 1) * sizeFMwPadding_;
            step_ = 0;
            stepCol_ = 0;
        }

        tailEventBuffer_ = false; // event terminate
    }


}

// check whether can send data to array
// 1. heatPtr is above tailPtr a conv window
bool Buffer::sendRdy() const 
{
    return (dataNum_ >= (sizeK_ - 1) * sizeFMwPadding_ + sizeK_) && !tailEventBuffer_;
}

// return the latency for sending data from buffer to tile
int Buffer::sendTime() const
{
    return buffer2tileLatency_;
}

// shedule ready time for the input data
void Buffer::setOutTime(long long int clockTime, int latency)
{
    // schedule the time for tailPtr
    tailEventTime_ = clockTime + buffer2tileLatency_ + latency;
}

// send data to array - constrained by bus width
// each data is 8 or 4 bit
std::vector<int> Buffer::sendData()
{
    std::vector<std::vector<int>> dataConvWindow;

    for (size_t i = 0; i < sizeK_; ++i) {
        for (size_t j = 0; j < sizeK_; ++j) {
            int idx = (tailPtr_ + j + i * sizeFM_) % bufferSize_;
            dataConvWindow.push_back(bufferData_[idx]);
        }
    }

    // Flatten data in the Conv Window to 1D
    std::vector<int> flattenData;
    for (auto const& v: dataConvWindow) {
        flattenData.insert(flattenData.end(), v.begin(), v.end());
    }

    // Schedule sending event
    tailEventBuffer_ = true;

    return flattenData;
}

// Debug code for test only
void Buffer::visTest() const
{
    for (size_t i = 0; i < bufferSize_; ++i) {
        std::cout << i << ": ";
        for (size_t j = 0; j < bufferDepth_; ++j) {
            std::cout << (unsigned)bufferData_[i][j] << " ";
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

void Buffer::bufferConfig() const 
{
    printf("====================\n");
    std::cout << "Buffer Size: " << bufferData_.size() << std::endl;
    std::cout << "Channel Depth:" << bufferData_[0].size() << std::endl;
}

void Buffer::showDataNum() const 
{
    std::cout << "Buffer Data Number: " << dataNum_ << std::endl;
    std::cout << "Buffer Head Ptr: " << headPtr_ << std::endl;
    std::cout << "Buffer Tail Ptr: " << tailPtr_ << std::endl;
}

// Destructor
Buffer::~Buffer()
{
    // std::cout << "Destructor" << std::endl;
}
