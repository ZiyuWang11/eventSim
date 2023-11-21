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
                size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride, size_t padding, bool singlePadding = false); // Buffer

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

    virtual void checkBuffer() const {buffer.showDataNum();}

    // Not used methods
    virtual void setInputFirst(std::vector<int> data) {};

    virtual bool buffer2tile() const { return false;}

    virtual void setBuffer2Tile(long long int clockTime) {};

    virtual bool rdy4comp() const { return false; }

    virtual void setComp(long long int clockTime) {};

    virtual void layerConfig() const {};
    
};

#endif // LAYER_POOLING_H_