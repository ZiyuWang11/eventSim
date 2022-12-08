// layer_fc.h - Fully Connected Layer, inherit from LayerVMM

#ifndef LAYER_FC_H_
#define LAYER_FC_H_

#include <string>
#include <vector>
#include "layer_vmm.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

class LayerFC : public LayerVMM
{
public:
    LayerFC(int layerNum, std::string layerType, // Layer
              int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, // Tile
              size_t numOut, int lutNum, std::string af); // LUT

    virtual ~LayerFC() {};

    // Input Methods
    virtual bool sendRequest() const;
 
    virtual void setInputTime(long long int clockTime, LayerABC* prevLayer = NULL);

    virtual void setInputFirst(std::vector<int> data);

    virtual void setInput(LayerABC* prevLayer = NULL);

    virtual bool buffer2tile() const { return false;}

    virtual void setBuffer2Tile(long long int clockTime) {};

    // State Change Methods
    virtual void changeState(long long int clockTime);

    // Hardware Configuration
    virtual void layerConfig() const;
};

#endif // LAYER_FC_H_
