// buffer.h -- ring FIFO buffer
#ifndef BUFFER_H_
#define BUFFER_H_

#include <string>
#include <vector>

extern const int dataPrecision;
extern const int busWidth; 

class Buffer
{
private:
    // architecutre configuration
    size_t bufferSize_; // buffer(ring FIFO) size, excluding depth
    size_t bufferDepth_; // channel depth of each pixel
    size_t headPtr_; // head pointer of ring FIFO
    size_t tailPtr_; // tail pointer of ring FIFO
    // model configuration - which data to send
    size_t sizeFM_; // input feature map size, assume to be square
    size_t sizeK_; // conv kernel size (2D)
    size_t stride_; // stride of conv kernel
    // data
    size_t dataNum_; // number of valid data in the bufer
    std::vector<std::vector<int>> bufferData_;
    // evnet timing
    bool headEventBuffer_; // event for loading data
    long long int headEventTime_; // event for loading data
    bool tailEventBuffer_; // event for sending data
    long long int tailEventTime_; // event for sending data
    // data forwarding latency buffer -> array
    // convWindow size * channel depth * dataPrecision / busWidth
    int buffer2tileLatency_;

public:
    /* Constructor of the Buffer class
     * Initialize buffer by the system configuration
     * Set head and tail pointer
     * Set number of clock cycles to pass the data to tile
     */
    Buffer();

    // Constructor
    Buffer(size_t bufferSize, size_t bufferDepth, size_t sizeFM, size_t sizeK, size_t stride);

    ~Buffer();

    //////////////////////////
    //                      //
    // Data loading methods //
    //                      //
    ////////////////////////// 
    
    /* Check buffer status
     * If the buffer is not full, and not loading data load data from previous layer or memory
     * This method will be called by the owner layer sendRequest() method
     */

    bool loadRdy() const;

    /* Set when the input data will be ready in the buffer
     * When the data is ready, the buffer moves its head pointer
     * Thie method will take the current time as an input, addition to the event time to get the ready time
     * This method will be called by the owner layer setInputTime() method
     */

    void setTime(long long int clockTime, int latency = 0); 

    /* Load data from previous layer, the latency is constraint by the bus width
     * The ordinary buffer operation are split into two phases
     * (i) Write data into buffer when get it at time T
     * (ii) Move the head pointer at event ready time T + T_{event}, which is defined by setTime()
     * loadData() only covers step (i), it will be called by the owner layer setInput() method
     */
    void loadData(std::vector<int> data);

    /* A check time function to self-control the state
     * Once an event is scheduled, check time every clock until event executed
     * This method operate abovementioned step (ii)
     */
    void movePtr(long long int clockTime);

    //////////////////////////
    //                      //
    // Data sending methods //
    //                      //
    ////////////////////////// 
    
    /* Check if buffer has data for a conv window and not sending data
     * If so, this will return true
     * This method will be called by the owner layer buffer2tile() method
     */

    bool sendRdy() const; // true when input for a VMM is ready

    /* Return the time to pass the input data from buffer to tile
     * This time is a constant, use a reference at layer level maybe more efficient
     * Change it latter if needed
     */

    int sendTime(); 

    /* Return the input data send to array
     * This method will be called by the owner layer setBuffer2tile() method
     * The vector will later to set the input register of tile
     */
    std::vector<std::vector<int>> sendData();

    // Debug code
    void visTest() const;
};
#endif //BUFFER_H_
