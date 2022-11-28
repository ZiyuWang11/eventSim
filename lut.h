// lut.h - header file of lookup table
#ifndef LUT_H_
#define LUT_H_

#include <iostream>

class LUT
{
private:
    const size_t lutSize_;
    // number of data to get from tile
    const size_t lutNum_;
    int* lutTable_;

public:
    /* Constructor, initilize LUT based on activation function,
     * Scaling factor and Batchnorm function
     * LUT can be an array our std::vector
     * Vector is more convinient but array more efficient
     */
    // Default constructor
    LUT();

    // Constructor - initialize LUT
    LUT(size_t lutSize = 256, size_t lutNum = 1);

    // Destructor
    ~LUT();

    ////////////////////
    //                //
    // Output methods //
    //                //
    ////////////////////

    /* Ouput methods will be called by the owner layer outData() 
     * and outTime() methods, returns data to pass to next buffer 
     * and the latency for this process.
     * outData() call Tile first, Tile will convert VMM result to a
     * multi-bit index. Then call compute() method in LUT to convert 
     * the result. Final result stored in std::vector<int>
     * LUT is stateless, it only returns results
     */

    // Get Latency for conversion and forwarding
    int getTime() const;

    // Get data from tile and conversion
    int* compute(float* dataIn) const;

};

#endif // LUT_H_
