// layer_pooling.h - pooling layer header file

#ifndef LAYER_POOLING_H_
#define LAYER_POOLING_H_

#include <string>
#include <vector>
#include "buffer.h"
#include "pooling.h"
#include "layerabc.h"

extern const int dataPrecision;
extern const int busWidth;

class LayerPooling : public LayerABC
{
private:
    Buffer buffer;
    Pooling pooling;

public:
    LayerPooling(int layerNum, std::string layerType, // Layer
                std::string poolingType, // Pooling
                size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding); // Buffer

    ~LayerPooling() {};

    // Input Methods
    virtual bool sendRequest() const;
 
    virtual void setInputTime(long long int clockTime, LayerABC* prevLayer = NULL);

    virtual void setInput(LayerABC* prevLayer = NULL);

    // Output response methods
    virtual bool getRequest() const;

    virtual std::vector<int> outData();

    virtual int outTime() const;

    virtual void setOutTime(long long int clockTime);

    // State Change Methods
    virtual void changeState(long long int clockTime);
};

#endif // LAYER_POOLING_H_