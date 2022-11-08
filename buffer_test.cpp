// buffer_test.cpp - test buffer class
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"

int timeGlobal = 0;
const int busWidth = 64;
const int dataPrecision = 8;

int main()
{
    size_t bufferSize = 16;
    size_t bufferDepth = 2;

    Buffer test_buffer(bufferSize, bufferDepth);

    return 0;
}
