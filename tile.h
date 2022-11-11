// tile.h - header file for CIM tile
#ifndef TILE_H_
#define TILE_H_

#include <string>
#include <Eigen/Dense>

extern int timeGlobal;
extern const int dataPrecision;
extern const int busWidth; 

class Tile 
{
private:
    // State
    // compute - array working, no data in input/output register
    // compute_done - array done, output register pop data to LUT, 
    //                input register can load data from buffer
    // done - all data send to LUT, can load data or compute
    int inputState_;
    int arrayState_;
    
    // NN model configuration
    size_t sizeK_;
    size_t numK_;
    size_t channelDepth_;

    // Architecture configuration
    int devicePrecision_;
    size_t arraySizeX_; // width
    size_t arraySizeY_; // height
    size_t numADC_;

    size_t arrayNumX_;
    size_t arrayNumY_;

    // Latency configuration
    int latencyMVM_;
    const static int concHTree_ = 1;
    const static int addHTree_ = 1;
    
    // Weight
    Eigen::MatrixXf weight_;
    // Input register
    Eigen::VectorXf input_;
    // Output register
    Eigen::VectorXf output_;

public:
    //friend class Buffer;
    enum {isRdy, notRdy};
    enum {compute, compute_done, done};
    
    // default Constructor
    Tile();
    
    // Constructor
    Tile(size_t sizeK, size_t numK, size_t channelDepth, int devicePrecision, size_t arraySizeX, size_t arraySizeY,size_t numADC, const Eigen::MatrixXf& weight);

    // set latency for a layer MVM
    void setLatency();

    // check load
    bool inputRdy() const {return inputState_ == notRdy;}
    
    // check compute
    bool compRdy() const {return arrayState_ == done;}

    // check result
    bool outputRdy() const {return arrayState_ == compute_done;}
    
    // Dataloader
    void loadData(std::vector<std::vector<uint8_t>> data);

    // MVM computation
    void computeMVM();

    // get output to LUT
    std::vector<uint8_t> getOutput();
 
    // Record event - load/send data, buffer full
    // Simple print now, update it when event table is ready
    void eventWrapper(int eventTime, std::string& event) const;

    // Debug code
    void visTest() const;
    
    // Destructor
    ~Tile();
};
#endif //TILE_H_
