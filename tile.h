// tile.h - header file for CIM tile
#ifndef TILE_H_
#define TILE_H_

#include <string>
#include <Eigen/Dense>

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
    size_t latencyVMM_;
    const static int concHTree_ = 1;
    const static int addHTree_ = 1;

    // event timing
    bool inputEventTile_; // event for getting input from buffer
    long long int inputEventTime_; // schedule event for input
    bool arrayEventTile_; // event for computation
    long long int arrayEventTime_; // schedule event for array
    bool outputEventTile_; // event for ouput Tile -> LUT
    long long int outputEventTime_; // schedule event for output
    
    // Weight
    Eigen::MatrixXf weight_;
    // Input register
    Eigen::VectorXf input_;
    // Output register
    Eigen::VectorXf output_;

public:
    enum INPUT_STATE {isRdy, notRdy};
    enum ARRAY_STATE {compute, compute_done, done};
    
    /* Constructor to initialize the tile configuration 
     * A layer contains all tiles and H-Tree connections
     * Calculated the execution of VMM from hardware configuration - ADC number, array size, etc.
     * Map the quantized weight according to conductance range to an Eigen::Matrix
     */

    // default Constructor
    Tile();
    
    // Constructor
    Tile(size_t sizeK, size_t numK, size_t channelDepth, int devicePrecision, size_t arraySizeX, size_t arraySizeY,size_t numADC, const Eigen::MatrixXf& weight);

    // Destructor
    ~Tile();

    ////////////////////
    //                //
    // State Controll //
    //                //
    ////////////////////

    /* State controll methods for input register
     * Method setInputTime() record the event time for input register state changing
     * Method changeInputState() check the global clock and changes state accordingly
     * Initial state is notRdy
     * (i) If the data is requested from the previous module, schedule the event based on latency
     *     At the event time, change input register state from notRdy to isRdy
     * (ii) If VMM computation starts, schedule the state change based on computatino latency
     *      At the time of computation completion, change inpu register state from isRdy to notRdy
     * Event to be scheduled: (i) input data write to register latency (ii) VMM execution latency
     */

    /* State controll methods for array and output register
     * Method setArrayTime() record the event time for array state changing
     * Method changeArrayState() check the global clock and changes state accordingly
     * Initial state is done
     * (i) done -> comp: When input register state changes from notRdy, array state change from done to comp
     *                   instantantly. In the Meantime, schedule the event for comp -> comp_done
     * (ii) comp -> comp_done: Based on the constant latency, once arrive the scheduled time, change.
     * (iii) comp_done -> done: Related to all output data are sent, may change before or after input register
     *                          state change. When the layer got request from next layer, data in the 
     *                          output register will go through the LUT. Schedule the evnet time 
     *                          for all output data are sent, then change the state from comp_done to done.
     *                          If input register is notRdy, this will be the initial state.
     *                          If input register isRdy earlier, change comp_done -> done -> comp directly
     * Event to be scheduled: (i) VMM execution latency (ii) output data latnecy
     */

    void changeState(long long int clockTime);

    ////////////////////////// 
    //                      //
    // Data Loading Methods //
    //                      //
    ////////////////////////// 

    /* Check the state of the input register of arrays
     * If the input is not ready, that means previous input has been executed
     * Then it is ready to get next input, and request data from buffer of previous layer
     * This method will be called by the owner layer buffer2tile() method for conv/pooling
     * Called by the owner layer sendRequest() method for FC
     */

    bool loadRdy() const;  // true for ready to load data

    void setInTime(long long int clockTime, int latency);

    /* Load the data from Buffer.sendData() method to the input register Eigen::VectorXf input_
     * Data will be written instantly, but only can be used when the inputState_ == isRdy
     * The state change is determined by the event time, find more detail in State Change methods
     * This method will be called by the owner layer setBuffer2Tile() method
     */ 
    void loadData(std::vector<std::vector<int>> data);

    ///////////////////////// 
    //                     //
    // Computation Methods //
    //                     //
    ///////////////////////// 
 
    /* Check if the tile is ready for computation
     * Check if input isRdy and array is done
     * This method will be called by the owner layer rdy4Comp() method
     */

    bool compRdy() const; // true if ready for MVM computation

    void setCompTime(long long int clockTime);

    /* Once the input register state changes from notRdy to isRdy
     * The array state can change from done to comp instantly
     * A corner case is output still holds data, at comp_done state, it will change to comp when all data are sent
     * Computation result will be returned instantly and written into output registers Eigen::VectorXf output_
     * But it can only be used after the latency for computation, at that time array state change from comp to comp done
     * Find more information about state in State Controll methods
     * Thie method will be called by onwer execution() method
     */
 
    void computeVMM();

    ////////////////////////// 
    //                      //
    // Data sending Methods //
    //                      //
    ////////////////////////// 

    // set latency for a layer MVM
    // void setLatency(); MOVE TO CONSTRUCTOR

    /* Check if the result is ready in the output register
     * If the output is ready, this layer can reponse to next layer's request 
     * This method will be called by the owner layer getRequest() method
     */

    bool outputRdy() const;
    
    void setOutTime(long long int clockTime, int latency);
    
    /* Get the output data and convert from Eigen::VectorXf to std::vector<int>
     * This vector is the index to LUT, for 8-bit, it must be convert to [0, 255]
     * This method will be called by the outData() of the owner layer method
     * And pass the data to LUT class
     */
    std::vector<int> getOutput();

    // Debug code
    void visTest() const;
    
};
#endif //TILE_H_
