// pooling.h - header file of pooling unit
#ifndef POOLING_H_
#define POOLING_H_

#include <vector>
#include <string>

extern const int dataPrecision;
extern const int busWidth;

class Pooling
{
private:
    const std::string poolingType_; // Max or Ave
    size_t channelDepth_;
    size_t filterSize_; // Pooling Filter Size
    // Latency to execute pooling function
    int outLatency_;

public:
    // Default Constructor
    Pooling();

    // Constructor - initialize Pooling Unit
    Pooling(size_t channelDepth, size_t filterSize, std::string poolingType = "Ave");

    // Destructor
    ~Pooling();

    // Get Latency for conversion and forwarding
    int getTime() const;

    // Outer dimension - depth, Inner dimension - window data
    std::vector<int> compute(std::vector<int> dataIn);
};

#endif // POOLING_H_