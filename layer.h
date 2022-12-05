// layer.h - header file for layer

#ifndef LAYER_H_
#define LAYER_H_

#include <cmath>
#include <vector>
#include <string>
#include <Eigen/Dense>
#include "buffer.h"
#include "tile.h"
#include "lut.h"

extern const int dataPrecision;
extern const int busWidth;
extern const int memLatency;

class Layer
{
private:
    Buffer buffer;
    Tile tile;
    LUT lut;
    // other private representation to be developed

public:
    /* Constructor of the Layer class
     * call other constructors to initialize sub-modules
     */
    Layer(size_t bufferSize, size_t depth, size_t sizeFM, size_t sizeK, size_t stride, size_t numK, int devicePrecision, size_t arraySizeX, size_t arraySizeY, size_t numADC, const Eigen::MatrixXf& weight, int lutNum, std::string af);

    ~Layer();

    /////////////////////////////
    //                         //
    // Data requesting methods //
    //                         //
    ///////////////////////////// 

    /* Check if the layer is ready to request data from the previous layer
     * Call sub-module method to check the status
     * For layer (conv, pool) with buffer, call Buffer method to check pointer position
     * For layer (fc) without buffer, call Array Tile method to check input register
     */

    bool sendRequest() const; // true for sending request

    /* If get response from the previous layer, set input value and input ready time
     * This method will check the state of the previous layer, may need a pointer as input
     * The first layer may need to consider off-chip memory bandwidth
     * This method may replaced with control signal in the main function in the future
     * Previous layer will pass the input data and input setup time
     */
  
    bool getResponse(Layer* prevLayer = NULL) const; // true for get input data from previous layer

    /* Set when the input is ready, at the ready time, sub-module will change status
     * The ready time will pass to sub-modules by calling their methods
     * Input ready time is the current clock cycle plus the number of clocks to pass the data
     * Time to pass the data is usually a constant
     */ 

    void setInputTime(long long int clockTime, Layer* prevLayer = NULL);

    /* This methods will run simultaneously with setInputTime()
     * The data will set at the time when the layer get response
     * But only change the state of the sub-module the input ready time
     * i.e. the data is written but not ready to use
     * By doing so to avoid holding data.
     */
 
    void setInput(Layer* prevLayer = NULL);

    //////////////////////////
    //                      //
    // Data sending methods //
    //                      //
    ////////////////////////// 

    /* When the next layer send a request, call this method
     * This method will check sub-module methods to find out if out put data is ready
     * When the out put data is ready, getRequest() will return true
     * The output data and time to pass the data will be sent simultaneously
     */
 
    bool getRequest() const; // true for data is ready to send to next layer

    /* return the output data in the vector format
     * this method will be called by the next layer
     * the outTime() will be called simulataneously
     */

    std::vector<int> outData();

    /* return the time to pass the output data to the next layer
     * this method will be called by the next layer
     * This time should be a constant, use a reference maybe more efficient
     * Change it later if needed
     */
 
    int outTime() const;

    /* Get the output latency from the LUT
     * Schedule a state change from compute done to done in Tile
     */
    void setOutTime(long long int clockTime);

    ///////////////////////////
    //                       //
    // Inter-layer Data Flow //  Following methods for conv layer only
    //                       //
    ///////////////////////////

    /* Control data from Buffer to Tile
     * Return true when input data are ready in buffer and array computation is done
     */

    bool buffer2tile() const; // ture when input data is ready for Buffer -> Tile 
   
    /* Set input data and input ready time for tile
     * time to pass the data is a constant private member
     * The tile class will check clock to change it state
     */

    void setBuffer2Tile(long long int clockTime);

    //////////////////////////
    //                      //
    // Computation Controll //
    //                      //
    //////////////////////////

    /* This method check the input register and array state to see if it is ready for comuption
     * If it is ready, schedule the computation time and run computation
     */
    bool rdy4comp() const;

    /* This method set the event time for computation using Tile method
     * States of input register and array will be changed when computation is done
     */
    void setComp(long long int clockTime);

    ///////////////////
    //               //
    // State Control //
    //               //
    ///////////////////

    /* Move pointer of Buffer and Change state of Tile
     * call Buffer.movePtr() and Tile.changeState methods
     */
  
    void changeState(long long int clockTime);
};   

#endif // LAYER_H_
