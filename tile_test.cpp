// tile_test.cpp - test single layer tile function


#include <iostream>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include "tile.h"

const int busWidth = 64;
const int dataPrecision = 8;

int main()
{
    long long int clock = 1;
    int latencyIn = 10;
    int latencyOut = 50;

    Eigen::MatrixXf weight = Eigen::MatrixXf::Ones(27, 16);
    Tile test_tile(3, 16, 3, 4, 128, 128, 4, weight);

    while (clock < 500) {
        // Try to load data to input register
        if (test_tile.loadRdy()) {
            int value = (int)clock;
            std::vector<std::vector<int>> data(9, std::vector<int>(3, value));
            test_tile.loadData(data);
            test_tile.setInTime(clock, latencyIn);
        }

        // Try Computation
        if (test_tile.compRdy()) {
            test_tile.computeVMM();
            test_tile.setCompTime(clock);
        }

        // Try Get output
        if (test_tile.outputRdy()) {
            std::vector<int> output = test_tile.getOutput();
            test_tile.setOutTime(clock, latencyOut);
        }

        // Check and change state
        test_tile.changeState(clock);
        
        // Visualize state
        if (clock % 50 == 0) { 
            std::cout << "=======Time: " << clock << "=======\n";
            test_tile.visTest();
        }

        ++clock;
    }
    //test_tile.visTest();

    // get input data
    // change data soure to the friend class Buffer
    //std::vector<std::vector<uint8_t>> inData(9, std::vector<uint8_t>(3,1));
    //test_tile.loadData(inData);
    //test_tile.visTest();

    // VMM computation
    //test_tile.computeMVM();
    //test_tile.visTest();

    return 0;
}
