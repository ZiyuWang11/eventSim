// layerabc.h - abstract layer class
// layerabc -> layer_poolingabc and layer_vmmabc

#ifndef LAYERABC_H_
#define LAYERABC_H_

#include <string>
#include <vector>

/* Abstract Layer class
 * Only have Layer number and type as parameter
 * Son Classes - LayerPoolingABC, LayerVMMABC
 * Grandson Classes - LayerAveragePooling, LayerMaxPooling,
 *                    LayerConv, LayerFC
 * Constructor and Virutual Destructor
 */

class LayerABC
{
private:
    int layerNum_;
    std::string layerType_;

public:
    LayerABC(int layerNum, const std::string layerType);

    virtual ~LayerABC() {};

    /* Pure Virtual Function for Layer-wise Communication
     * Based on Layer Sub-modules
     * No Defination in Abstract Class
     */
  
    // Input request methods
    virtual bool sendRequest() const = 0;
 
    virtual void setInputTime(long long int clockTime, LayerABC* prevLayer = NULL) = 0;

    virtual void setInput(LayerABC* prevLayer = NULL) = 0;

    virtual void setInputFirst(std::vector<int> data) =0;

    // Output response methods
    virtual bool getRequest() const = 0;

    virtual std::vector<int> outData() = 0;

    virtual int outTime() const = 0;

    virtual void setOutTime(long long int clockTime) =0;
    
    // Buffer -> Tile Methods
    virtual bool buffer2tile() const =0;

    virtual void setBuffer2Tile(long long int clockTime) =0;

    // Tile Computation Methods
    virtual bool rdy4comp() const =0;

    virtual void setComp(long long int clockTime) =0;

    // State change methods
    virtual void changeState(long long int clockTime) =0;
};

#endif // LAYERABC_H_
