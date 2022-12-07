// layervmmabc.h - father of conv and fc layer

#ifndef LAYER_VMM_H_
#define LAYER_VMM_H_

#include <string>
#include <vector>
#include "tile.h"
#include "lut.h"
#include "layerabc.h"

class LayerVMM : public LayerABC
{
protected:
    Tile tile;
    LUT lut;

public:
	LayerVMM(int layerNum, std::string layerType, // Layer
                int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, // Tile
                size_t numOut, int lutNum, std::string af); // LUT

    virtual ~LayerVMM() {};

    /* Conv has Buffer while FC not, input methods vary.
     * No Defination for input setup methods
     * For Convenience, Add An InputSet for the First Layer
     */

    // Input Methods    
    // virtual void setInputFirst(std::vector<int> data) =0; // Pure Virtual Function

    // Output response methods
    virtual bool getRequest() const;

    virtual std::vector<int> outData();

    virtual int outTime() const;

    virtual void setOutTime(long long int clockTime);

    // Tile Computation Methods
    virtual bool rdy4comp() const;

    virtual void setComp(long long int clockTime);
};

#endif // LAYER_VMM_H_
