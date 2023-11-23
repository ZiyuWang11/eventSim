// layer_conv.h - Convolution Layer, inherit from LayerVMM

#ifndef LAYER_CONV_H_
#define LAYER_CONV_H_

#include <string>
#include <vector>
#include "buffer.h"
#include "layer_vmm.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

class LayerConv : public LayerVMM
{
private:
    Buffer buffer;

public:
    LayerConv(int layerNum, std::string layerType, // Layer
              int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, // Tile
              size_t numOut, int lutNum, std::string af, // LUT
              size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding, bool singlePadding = false); // Buffer

    virtual ~LayerConv() {};

    // Input Methods
    virtual bool sendRequest() const;
 
    virtual void setInputTime(long long int clockTime, LayerABC* prevLayer = NULL);

    virtual void setInputFirst(std::vector<int> data);

    virtual void setInput(LayerABC* prevLayer = NULL);

    // Buffer -> Tile Methods
    virtual bool buffer2tile() const;

    virtual void setBuffer2Tile(long long int clockTime);

    // State Change Methods
    virtual void changeState(long long int clockTime);

    // Hardware Configuration
    virtual void layerConfig() const;
};

#endif // LAYER_CONV_H_
