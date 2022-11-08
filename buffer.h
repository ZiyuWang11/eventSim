// buffer.h -- ring FIFO buffer
#ifndef BUFFER_H_
#define BUFFER_H_

#include <string>
#include <vector>

extern int timeGlobal;
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
    size_t sizeFM_;
    size_t sizeK_;
    // data
    size_t dataNum_;
    std::vector<std::vector<uint8_t>> bufferData_;

public:
    // Default constructor
    Buffer();

    // Constructor
    Buffer(size_t bufferSize = 0, size_t bufferDepth = 0, size_t sizeFM = 0, size_t sizeK = 0);

    // check buffer status
    bool isFull() const;

    // check array for sending data
    bool sendRdy(bool arrayRdy = 1) const;

    // send data to array
    // constrained by bus width
    std::vector<std::vector<uint8_t>> sendData();

    // check previous layer for loading data
    bool loadRdy() const;

    // load data from previous layer
    // constrained by bus width
    void loadData(std::vector<uint8_t> data);

    // Record event - load/send data, buffer full
    // Simple print now, update it when event table is ready
    void eventWrapper(int eventTime, std::string& event) const;

    // Debug code
    void visTest() const;

    // Destructor
    ~Buffer();
};
#endif //BUFFER_H_
