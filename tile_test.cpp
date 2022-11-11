// tile_test.cpp - test single layer tile function


#include <iostream>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include "tile.h"

int timeGlobal = 0;
const int busWidth = 64;
const int dataPrecision = 8;

int main()
{
    Eigen::MatrixXf weight = Eigen::MatrixXf::Ones(27, 16);
    Tile test_tile(3, 16, 3, 4, 128, 128, 4, weight);
    test_tile.visTest();

    // get input data
    // change data soure to the friend class Buffer
    std::vector<std::vector<uint8_t>> inData(9, std::vector<uint8_t>(3,1));
    test_tile.loadData(inData);
    test_tile.visTest();

    // VMM computation
    test_tile.computeMVM();
    test_tile.visTest();

    return 0;
}
