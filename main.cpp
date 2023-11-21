// main.cpp - test code for LeNet and AlexNet

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "layer_conv.h"
#include "layer_fc.h"
#include "layer_pooling.h"
#include "testCases.h"

const int dataPrecision = 8;
const int busWidth = 128;
const int memLatency = 20;

// Test LeNet and AlexNet, no pooling layer at this moment
// void testLeNetPooling(bool debug = false);
// void testLeNet(bool debug = false);
// void testAlexNet(bool debug = false);
// void testAlexNetFake(bool debug = false);
// void testCode(bool debug, int layerNum, LayerABC** layer_test, int inputWidth, int inputDepth);


int main() {
    // If debug is true, will print debug code
    bool debug = true;

    // Change the Reference - 1) LeNet, 2) AlexNet, 3) Vgg, 4) ResNet
    void (*testFunction) (bool) = &testVgg;

    (*testFunction)(debug);

    return 0;
}
