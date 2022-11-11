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
    Tile test_tile;

    Eigen::MatrixXf weight = Eigen::MatrixXf::Random(256, 256);
    Tile test_tile1(3, 16, 3, 4, 128, 128, 4, weight);
    return 0;
}
