#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include "layer_conv.h"
#include "layer_fc.h"
#include "layer_pooling.h"



void testCode(bool debug, size_t layerNum, LayerABC** layer_test, size_t inputWidth, size_t inputDepth, size_t inputNum) {
    printf("====Architecuture Configuration====\n");
    for (int i = 0; i < layerNum; ++i) {
        layer_test[i]->layerConfig();
    }

    // Initialize clock
    long long int clock = 1;

    // Initialize data to be processed
    size_t inPixel = 0;
    size_t outCount = 0; 
    
    while (outCount < inputNum) {
        // Request Data from back to forward
        if (layer_test[layerNum-1]->getRequest()) {
            layer_test[layerNum-1]->setOutTime(clock);
            std::vector<int> output = layer_test[layerNum-1]->outData();
            ++outCount;
        }

        for (int i = layerNum - 1; i > 0; --i) {
            // Check if data can be pass through two layers
            if (layer_test[i]->sendRequest() && layer_test[i-1]->getRequest()) {
                if (debug) std::cout << "Send data from Layer " << i << " to Layer " << i+1 << " at clock " << clock << std::endl;
                // Set Input and Input Time
                // Time and Data are get from previous layer
                layer_test[i]->setInput(layer_test[i-1]);
                layer_test[i]->setInputTime(clock, layer_test[i-1]);
                layer_test[i-1]->setOutTime(clock);
            }
        }

        // Check if load data from memory for the first layer
        if (layer_test[0]->sendRequest() && (inPixel < inputWidth * inputWidth * inputNum)) {
            if (debug) std::cout << "Send data from Memory to Layer 1 at clock " << clock << std::endl;
            // Use random number as input at this moment
            int value = (int)clock;
            std::vector<int> data(inputDepth, value);
            layer_test[0]->setInputFirst(data);
            layer_test[0]->setInputTime(clock);
            ++inPixel;
        }

        // Process Data 
        for (int i = 0; i < layerNum; ++i) {
            // Buffer -> Tile
            if (layer_test[i]->buffer2tile()) {
                if (debug) std::cout << "Layer " << i+1 << " sends data from buffer to tile at clock " << clock << std::endl;
                layer_test[i]->setBuffer2Tile(clock);
            }
           
            // VMM
            if (layer_test[i]->rdy4comp()) {
                if (debug) std::cout << "Layer " << i+1 << " executes VMM at clock " << clock << std::endl;
                layer_test[i]->setComp(clock);
            }
 
        }

        // State update
        for (int i = 0; i < layerNum; ++i) {
            layer_test[i]->changeState(clock);
        }

        ++clock;
    }
    
    printf("==========================\n");
    printf("Terminate at Clock %lld\n", clock);
    printf("==========================\n");

}

// LeNet Test - 2 Conv + 3 FC + 2 Pooling
void testLeNet(bool debug) { 
    // Input Configuration
    size_t inputNum = 10;
    // Buffer Configuration for inifinite buffer
    size_t bufferSize[4] = {10000, 10000, 10000, 10000};
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 4;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";
    
    // Layer 1 Configuration
    // NN Configuration
    size_t depth1 = 3;
    size_t sizeFM1 = 28;
    size_t sizeK1 = 5;
    size_t stride1 = 1;
    size_t padding1 = 2;
    size_t numK1 = 6;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 Configuration - Pooling
    size_t depth2 = 6;
    size_t sizeFM2 = 28;
    size_t sizeK2 = 2;
    size_t stride2 = 2;
    size_t padding2 = 0;

    // Layer 3 configuration
    size_t sizeFM3 = 14;
    size_t sizeK3 = 5;
    size_t depth3 = 6;
    size_t stride3 = 1;
    size_t padding3 = 0;
    size_t numK3 = 16;
    Eigen::MatrixXf weight3 = Eigen::MatrixXf::Ones(sizeK3*sizeK3*depth3, numK3);

    // Layer 4 Configuration - Pooling
    size_t depth4 = 16;
    size_t sizeFM4 = 10;
    size_t sizeK4 = 2;
    size_t stride4 = 2;
    size_t padding4 = 0;

    // Layer 5 FC configuration
    size_t outNum5 = 120;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(numK3*5*5, outNum5);

    // Layer 6 FC configuration
    size_t outNum6 = 84;
    Eigen::MatrixXf weight6 = Eigen::MatrixXf::Ones(outNum5, outNum6);

    // Layer 7 FC configuration
    size_t outNum7 = 10;
    Eigen::MatrixXf weight7 = Eigen::MatrixXf::Ones(outNum6, outNum7);

    // Initialize Layers
    const int layerNum = 7;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, bufferSize[0], depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerPooling(2, "Pooling", "Ave", bufferSize[1], depth2, sizeFM2, sizeK2, stride2, padding2);
    layer_test[2] = new LayerConv(3, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight3, numK3, lutNum, af, bufferSize[2], depth3, sizeFM3, sizeK3, stride3, padding3);
    layer_test[3] = new LayerPooling(4, "Pooling", "Ave", bufferSize[3], depth4, sizeFM4, sizeK4, stride4, padding4);
    layer_test[4] = new LayerFC(5, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, outNum5, lutNum, af);
    layer_test[5] = new LayerFC(6, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight6, outNum6, lutNum, af);
    layer_test[6] = new LayerFC(7, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight7, outNum7, lutNum, af);
   
    testCode(debug, layerNum, layer_test, sizeFM1, depth1, inputNum);
}

// AlexNet Test - 5 Conv + 3 FC + 3 Pooling
void testAlexNet(bool debug) { 
    // Input Configuration
    size_t inputNum = 2;
    // Buffer Configuration for inifinite buffer
    size_t bufferSize[8] = {10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000};
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 64;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";
    
    // Layer 1 Configuration
    size_t depth1 = 3;
    size_t sizeFM1 = 227;
    size_t sizeK1 = 11;
    size_t stride1 = 4;
    size_t padding1 = 0;
    size_t numK1 = 96;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 Configuration - Pooling
    size_t depth2 = 96;
    size_t sizeFM2 = 55;
    size_t sizeK2 = 3;
    size_t stride2 = 2;
    size_t padding2 = 0;

    // Layer 3 configuration
    size_t sizeFM3 = 27;
    size_t sizeK3 = 5;
    size_t depth3 = 96;
    size_t stride3 = 1;
    size_t padding3 = 2;
    size_t numK3 = 256;
    Eigen::MatrixXf weight3 = Eigen::MatrixXf::Ones(sizeK3*sizeK3*depth3, numK3);

    // Layer 4 Configuration - Pooling
    size_t depth4 = 256;
    size_t sizeFM4 = 27;
    size_t sizeK4 = 3;
    size_t stride4 = 2;
    size_t padding4 = 0;

    // Layer 5 configuration
    size_t sizeFM5 = 13;
    size_t sizeK5 = 3;
    size_t depth5 = 256;
    size_t stride5 = 1;
    size_t padding5 = 1;
    size_t numK5 = 384;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(sizeK5*sizeK5*depth5, numK5);

    // Layer 6 configuration
    size_t sizeFM6 = 13;
    size_t sizeK6 = 3;
    size_t depth6 = 384;
    size_t stride6 = 1;
    size_t padding6 = 1;
    size_t numK6 = 384;
    Eigen::MatrixXf weight6 = Eigen::MatrixXf::Ones(sizeK6*sizeK6*depth6, numK6);

    // Layer 7 configuration
    size_t sizeFM7 = 13;
    size_t sizeK7 = 3;
    size_t depth7 = 384;
    size_t stride7 = 1;
    size_t padding7 = 1;
    size_t numK7 = 256;
    Eigen::MatrixXf weight7 = Eigen::MatrixXf::Ones(sizeK7*sizeK7*depth7, numK7);

    // Layer 8 Configuration - Pooling
    size_t depth8 = 256;
    size_t sizeFM8 = 13;
    size_t sizeK8 = 3;
    size_t stride8 = 2;
    size_t padding8 = 0;

    // Layer 9 FC configuration
    size_t outNum9 = 4096;
    Eigen::MatrixXf weight9 = Eigen::MatrixXf::Ones(numK7*6*6, outNum9);

    // Layer 10 FC configuration
    size_t outNum10 = 4096;
    Eigen::MatrixXf weight10 = Eigen::MatrixXf::Ones(outNum9, outNum10);

    // Layer 11 FC configuration
    size_t outNum11 = 1000;
    Eigen::MatrixXf weight11 = Eigen::MatrixXf::Ones(outNum10, outNum11);

    // Initialize Layers
    const int layerNum = 11;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, bufferSize[0], depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerPooling(2, "Pooling", "Max", bufferSize[1], depth2, sizeFM2, sizeK2, stride2, padding2);
    layer_test[2] = new LayerConv(3, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight3, numK3, lutNum, af, bufferSize[2], depth3, sizeFM3, sizeK3, stride3, padding3);
    layer_test[3] = new LayerPooling(4, "Pooling", "Max", bufferSize[3], depth4, sizeFM4, sizeK4, stride4, padding4);
    layer_test[4] = new LayerConv(5, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, numK5, lutNum, af, bufferSize[4], depth5, sizeFM5, sizeK5, stride5, padding5);
    layer_test[5] = new LayerConv(6, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight6, numK6, lutNum, af, bufferSize[5], depth6, sizeFM6, sizeK6, stride6, padding6);
    layer_test[6] = new LayerConv(7, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight7, numK7, lutNum, af, bufferSize[6], depth7, sizeFM7, sizeK7, stride7, padding7);
    layer_test[7] = new LayerPooling(8, "Pooling", "Max", bufferSize[7], depth8, sizeFM8, sizeK8, stride8, padding8);
    layer_test[8] = new LayerFC(9, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight9, outNum9, lutNum, af);
    layer_test[9] = new LayerFC(10, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight10, outNum10, lutNum, af);
    layer_test[10] = new LayerFC(11, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight11, outNum11, lutNum, af);

    testCode(debug, layerNum, layer_test, sizeFM1, depth1, inputNum);

}

// VGG16 Test - 13 Conv + 3 FC + 5 Pooling
void testVgg(bool debug) {
    // Input Configuration
    size_t inputNum = 1;
    // Buffer Configuration for inifinite buffer
    size_t bufferSize[18] = {100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000};
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 64;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";

    // Layer 1 Configuration - Conv
    size_t depth1 = 3;
    size_t sizeFM1 = 224;
    size_t sizeK1 = 3;
    size_t stride1 = 1;
    size_t padding1 = 1;
    size_t numK1 = 64;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 Configuration - Conv
    size_t depth2 = 64;
    size_t sizeFM2 = 224;
    size_t sizeK2 = 3;
    size_t stride2 = 1;
    size_t padding2 = 1;
    size_t numK2 = 64;
    Eigen::MatrixXf weight2 = Eigen::MatrixXf::Ones(sizeK2*sizeK2*depth2, numK2);

    // Layer 3 Configuration - Pooling
    size_t depth3 = 64;
    size_t sizeFM3 = 224;
    size_t sizeK3 = 2;
    size_t stride3 = 2;
    size_t padding3 = 0;

    // Layer 4 Configuration - Conv
    size_t depth4 = 64;
    size_t sizeFM4 = 112;
    size_t sizeK4 = 3;
    size_t stride4 = 1;
    size_t padding4 = 1;
    size_t numK4 = 128;
    Eigen::MatrixXf weight4 = Eigen::MatrixXf::Ones(sizeK4*sizeK4*depth4, numK4);

    // Layer 5 Configuration - Conv
    size_t depth5 = 128;
    size_t sizeFM5 = 112;
    size_t sizeK5 = 3;
    size_t stride5 = 1;
    size_t padding5 = 1;
    size_t numK5 = 128;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(sizeK5*sizeK5*depth5, numK5);

    // Layer 6 Configuration - Pooling
    size_t depth6 = 128;
    size_t sizeFM6 = 112;
    size_t sizeK6 = 2;
    size_t stride6 = 2;
    size_t padding6 = 0;

    // Layer 7 Configuration - Conv
    size_t depth7 = 128;
    size_t sizeFM7 = 56;
    size_t sizeK7 = 3;
    size_t stride7 = 1;
    size_t padding7 = 1;
    size_t numK7 = 256;
    Eigen::MatrixXf weight7 = Eigen::MatrixXf::Ones(sizeK7*sizeK7*depth7, numK7);

    // Layer 8 Configuration - Conv
    size_t depth8 = 256;
    size_t sizeFM8 = 56;
    size_t sizeK8 = 3;
    size_t stride8 = 1;
    size_t padding8 = 1;
    size_t numK8 = 256;
    Eigen::MatrixXf weight8 = Eigen::MatrixXf::Ones(sizeK8*sizeK8*depth8, numK8);

    // Layer 9 Configuration - Conv
    size_t depth9 = 256;
    size_t sizeFM9 = 56;
    size_t sizeK9 = 3;
    size_t stride9 = 1;
    size_t padding9 = 1;
    size_t numK9 = 256;
    Eigen::MatrixXf weight9 = Eigen::MatrixXf::Ones(sizeK9*sizeK9*depth9, numK9);

    // Layer 10 Configuration - Pooling
    size_t depth10 = 256;
    size_t sizeFM10 = 56;
    size_t sizeK10 = 2;
    size_t stride10 = 2;
    size_t padding10 = 0;

    // Layer 11 Configuration - Conv
    size_t depth11 = 256;
    size_t sizeFM11 = 28;
    size_t sizeK11 = 3;
    size_t stride11 = 1;
    size_t padding11 = 1;
    size_t numK11 = 512;
    Eigen::MatrixXf weight11 = Eigen::MatrixXf::Ones(sizeK11*sizeK11*depth11, numK11);

    // Layer 12 Configuration - Conv
    size_t depth12 = 512;
    size_t sizeFM12 = 28;
    size_t sizeK12 = 3;
    size_t stride12 = 1;
    size_t padding12 = 1;
    size_t numK12 = 512;
    Eigen::MatrixXf weight12 = Eigen::MatrixXf::Ones(sizeK12*sizeK12*depth12, numK12);

    // Layer 13 Configuration - Conv
    size_t depth13 = 512;
    size_t sizeFM13 = 28;
    size_t sizeK13 = 3;
    size_t stride13 = 1;
    size_t padding13 = 1;
    size_t numK13 = 512;
    Eigen::MatrixXf weight13 = Eigen::MatrixXf::Ones(sizeK13*sizeK13*depth13, numK13);

    // Layer 14 Configuration - Pooling
    size_t depth14 = 512;
    size_t sizeFM14 = 28;
    size_t sizeK14 = 2;
    size_t stride14 = 2;
    size_t padding14 = 0;

    // Layer 15 Configuration - Conv
    size_t depth15 = 512;
    size_t sizeFM15 = 14;
    size_t sizeK15 = 3;
    size_t stride15 = 1;
    size_t padding15 = 1;
    size_t numK15 = 512;
    Eigen::MatrixXf weight15 = Eigen::MatrixXf::Ones(sizeK15*sizeK15*depth15, numK15);

    // Layer 16 Configuration - Conv
    size_t depth16 = 512;
    size_t sizeFM16 = 14;
    size_t sizeK16 = 3;
    size_t stride16 = 1;
    size_t padding16 = 1;
    size_t numK16 = 512;
    Eigen::MatrixXf weight16 = Eigen::MatrixXf::Ones(sizeK16*sizeK16*depth16, numK16);

    // Layer 17 Configuration - Conv
    size_t depth17 = 512;
    size_t sizeFM17 = 14;
    size_t sizeK17 = 3;
    size_t stride17 = 1;
    size_t padding17 = 1;
    size_t numK17 = 512;
    Eigen::MatrixXf weight17 = Eigen::MatrixXf::Ones(sizeK17*sizeK17*depth17, numK17);

    // Layer 18 Configuration - Pooling
    size_t depth18 = 512;
    size_t sizeFM18 = 14;
    size_t sizeK18 = 2;
    size_t stride18 = 2;
    size_t padding18 = 0;

    // Layer 19 Configuration - FC
    size_t outNum19 = 4096;
    Eigen::MatrixXf weight19 = Eigen::MatrixXf::Ones(numK17*7*7, outNum19);

    // Layer 20 Configuration - FC
    size_t outNum20 = 4096;
    Eigen::MatrixXf weight20 = Eigen::MatrixXf::Ones(outNum19, outNum20);

    // Layer 21 Configuration - FC
    size_t outNum21 = 1000;
    Eigen::MatrixXf weight21 = Eigen::MatrixXf::Ones(outNum20, outNum21);

    // Initialize Layers
    const int layerNum = 21;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, bufferSize[0], depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerConv(2, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight2, numK2, lutNum, af, bufferSize[1], depth2, sizeFM2, sizeK2, stride2, padding2);
    layer_test[2] = new LayerPooling(3, "Pooling", "Max", bufferSize[2], depth3, sizeFM3, sizeK3, stride3, padding3);
    layer_test[3] = new LayerConv(4, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight4, numK4, lutNum, af, bufferSize[3], depth4, sizeFM4, sizeK4, stride4, padding4);
    layer_test[4] = new LayerConv(5, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, numK5, lutNum, af, bufferSize[4], depth5, sizeFM5, sizeK5, stride5, padding5);
    layer_test[5] = new LayerPooling(6, "Pooling", "Max", bufferSize[5], depth6, sizeFM6, sizeK6, stride6, padding6);
    layer_test[6] = new LayerConv(7, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight7, numK7, lutNum, af, bufferSize[6], depth7, sizeFM7, sizeK7, stride7, padding7);
    layer_test[7] = new LayerConv(8, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight8, numK8, lutNum, af, bufferSize[7], depth8, sizeFM8, sizeK8, stride8, padding8);
    layer_test[8] = new LayerConv(9, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight9, numK9, lutNum, af, bufferSize[8], depth9, sizeFM9, sizeK9, stride9, padding9);
    layer_test[9] = new LayerPooling(10, "Pooling", "Max", bufferSize[9], depth10, sizeFM10, sizeK10, stride10, padding10);
    layer_test[10] = new LayerConv(11, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight11, numK11, lutNum, af, bufferSize[10], depth11, sizeFM11, sizeK11, stride11, padding11);
    layer_test[11] = new LayerConv(12, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight12, numK12, lutNum, af, bufferSize[11], depth12, sizeFM12, sizeK12, stride12, padding12);
    layer_test[12] = new LayerConv(13, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight13, numK13, lutNum, af, bufferSize[12], depth13, sizeFM13, sizeK13, stride13, padding13);
    layer_test[13] = new LayerPooling(14, "Pooling", "Max", bufferSize[13], depth14, sizeFM14, sizeK14, stride14, padding14);
    layer_test[14] = new LayerConv(15, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight15, numK15, lutNum, af, bufferSize[14], depth15, sizeFM15, sizeK15, stride15, padding15);
    layer_test[15] = new LayerConv(16, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight16, numK16, lutNum, af, bufferSize[15], depth16, sizeFM16, sizeK16, stride16, padding16);
    layer_test[16] = new LayerConv(17, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight17, numK17, lutNum, af, bufferSize[16], depth17, sizeFM17, sizeK17, stride17, padding17);
    layer_test[17] = new LayerPooling(18, "Pooling", "Max", bufferSize[17], depth18, sizeFM18, sizeK18, stride18, padding18);
    layer_test[18] = new LayerFC(19, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight19, outNum19, lutNum, af);
    layer_test[19] = new LayerFC(20, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight20, outNum20, lutNum, af);
    layer_test[20] = new LayerFC(21, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight21, outNum21, lutNum, af);

    testCode(debug, layerNum, layer_test, sizeFM1, depth1, inputNum);


}

// ResNet18 Test - 17 Conv + 1 FC + 2 Pooling
void testResNet(bool debug) {
    // Input Configuration
    size_t inputNum = 1;
    // Buffer Configuration for inifinite buffer
    size_t bufferSize[19] = {100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000};
    // Tile Configuration
    int devicePrecision = 4;
    size_t arraySizeX = 128;
    size_t arraySizeY = 128;
    size_t numADC = 64;
    // LUT configuration
    int lutNum = 2;
    std::string af = "ReLU";

    // Layer 1 Configuration - Conv
    size_t depth1 = 3;
    size_t sizeFM1 = 227;
    size_t sizeK1 = 7;
    size_t stride1 = 2;
    size_t padding1 = 1;
    size_t numK1 = 64;
    Eigen::MatrixXf weight1 = Eigen::MatrixXf::Ones(sizeK1*sizeK1*depth1, numK1);

    // Layer 2 Configuration - Pooling
    size_t depth2 = 64;
    size_t sizeFM2 = 112;
    size_t sizeK2 = 3;
    size_t stride2 = 2;
    size_t padding2 = 1;

    // Layer 3 Configuration - Conv
    size_t depth3 = 64;
    size_t sizeFM3 = 56;
    size_t sizeK3 = 3;
    size_t stride3 = 1;
    size_t padding3 = 1;
    size_t numK3 = 64;
    Eigen::MatrixXf weight3 = Eigen::MatrixXf::Ones(sizeK3*sizeK3*depth3, numK3);

    // Layer 4 Configuration - Conv
    size_t depth4 = 64;
    size_t sizeFM4 = 56;
    size_t sizeK4 = 3;
    size_t stride4 = 1;
    size_t padding4 = 1;
    size_t numK4 = 64;
    Eigen::MatrixXf weight4 = Eigen::MatrixXf::Ones(sizeK4*sizeK4*depth4, numK4);

    // Layer 5 Configuration - Conv
    size_t depth5 = 64;
    size_t sizeFM5 = 56;
    size_t sizeK5 = 3;
    size_t stride5 = 1;
    size_t padding5 = 1;
    size_t numK5 = 64;
    Eigen::MatrixXf weight5 = Eigen::MatrixXf::Ones(sizeK5*sizeK5*depth5, numK5);

    // Layer 6 Configuration - Conv
    size_t depth6 = 64;
    size_t sizeFM6 = 56;
    size_t sizeK6 = 3;
    size_t stride6 = 1;
    size_t padding6 = 1;
    size_t numK6 = 64;
    Eigen::MatrixXf weight6 = Eigen::MatrixXf::Ones(sizeK6*sizeK6*depth6, numK6);

    // Layer 7 Configuration - Conv
    size_t depth7 = 64;
    size_t sizeFM7 = 56;
    size_t sizeK7 = 3;
    size_t stride7 = 2;
    size_t padding7 = 1;
    size_t numK7 = 128;
    Eigen::MatrixXf weight7 = Eigen::MatrixXf::Ones(sizeK7*sizeK7*depth7, numK7);

    // Layer 8 Configuration - Conv
    size_t depth8 = 128;
    size_t sizeFM8 = 28;
    size_t sizeK8 = 3;
    size_t stride8 = 1;
    size_t padding8 = 1;
    size_t numK8 = 128;
    Eigen::MatrixXf weight8 = Eigen::MatrixXf::Ones(sizeK8*sizeK8*depth8, numK8);

    // Layer 9 Configuration - Conv
    size_t depth9 = 128;
    size_t sizeFM9 = 28;
    size_t sizeK9 = 3;
    size_t stride9 = 1;
    size_t padding9 = 1;
    size_t numK9 = 128;
    Eigen::MatrixXf weight9 = Eigen::MatrixXf::Ones(sizeK9*sizeK9*depth9, numK9);

    // Layer 10 Configuration - Conv
    size_t depth10 = 128;
    size_t sizeFM10 = 28;
    size_t sizeK10 = 3;
    size_t stride10 = 1;
    size_t padding10 = 1;
    size_t numK10 = 128;
    Eigen::MatrixXf weight10 = Eigen::MatrixXf::Ones(sizeK10*sizeK10*depth10, numK10);

    // Layer 11 Configuration - Conv
    size_t depth11 = 128;
    size_t sizeFM11 = 28;
    size_t sizeK11 = 3;
    size_t stride11 = 2;
    size_t padding11 = 1;
    size_t numK11 = 256;
    Eigen::MatrixXf weight11 = Eigen::MatrixXf::Ones(sizeK11*sizeK11*depth11, numK11);

    // Layer 12 Configuration - Conv
    size_t depth12 = 256;
    size_t sizeFM12 = 14;
    size_t sizeK12 = 3;
    size_t stride12 = 1;
    size_t padding12 = 1;
    size_t numK12 = 256;
    Eigen::MatrixXf weight12 = Eigen::MatrixXf::Ones(sizeK12*sizeK12*depth12, numK12);

    // Layer 13 Configuration - Conv
    size_t depth13 = 256;
    size_t sizeFM13 = 14;
    size_t sizeK13 = 3;
    size_t stride13 = 1;
    size_t padding13 = 1;
    size_t numK13 = 256;
    Eigen::MatrixXf weight13 = Eigen::MatrixXf::Ones(sizeK13*sizeK13*depth13, numK13);

    // Layer 14 Configuration - Conv
    size_t depth14 = 256;
    size_t sizeFM14 = 14;
    size_t sizeK14 = 3;
    size_t stride14 = 1;
    size_t padding14 = 1;
    size_t numK14 = 256;
    Eigen::MatrixXf weight14 = Eigen::MatrixXf::Ones(sizeK14*sizeK14*depth14, numK14);

    // Layer 15 Configuration - Conv
    size_t depth15 = 256;
    size_t sizeFM15 = 14;
    size_t sizeK15 = 3;
    size_t stride15 = 2;
    size_t padding15 = 1;
    size_t numK15 = 512;
    Eigen::MatrixXf weight15 = Eigen::MatrixXf::Ones(sizeK15*sizeK15*depth15, numK15);

    // Layer 16 Configuration - Conv
    size_t depth16 = 512;
    size_t sizeFM16 = 7;
    size_t sizeK16 = 3;
    size_t stride16 = 1;
    size_t padding16 = 1;
    size_t numK16 = 512;
    Eigen::MatrixXf weight16 = Eigen::MatrixXf::Ones(sizeK16*sizeK16*depth16, numK16);

    // Layer 17 Configuration - Conv
    size_t depth17 = 512;
    size_t sizeFM17 = 7;
    size_t sizeK17 = 3;
    size_t stride17 = 1;
    size_t padding17 = 1;
    size_t numK17 = 512;
    Eigen::MatrixXf weight17 = Eigen::MatrixXf::Ones(sizeK17*sizeK17*depth17, numK17);

    // Layer 18 Configuration - Conv
    size_t depth18 = 512;
    size_t sizeFM18 = 7;
    size_t sizeK18 = 3;
    size_t stride18 = 1;
    size_t padding18 = 1;
    size_t numK18 = 512;
    Eigen::MatrixXf weight18 = Eigen::MatrixXf::Ones(sizeK18*sizeK18*depth18, numK18);

    // Layer 19 Configuration - Pooling
    size_t depth19 = 512;
    size_t sizeFM19 = 7;
    size_t sizeK19 = 7;
    size_t stride19 = 7;
    size_t padding19 = 0;

    // Layer 20 Configuration - FC
    size_t outNum20 = 1000;
    Eigen::MatrixXf weight20 = Eigen::MatrixXf::Ones(depth19, outNum20);

    // Initialize Layers
    const int layerNum = 20;
    LayerABC* layer_test[layerNum]; 
    layer_test[0] = new LayerConv(1, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight1, numK1, lutNum, af, bufferSize[0], depth1, sizeFM1, sizeK1, stride1, padding1);
    layer_test[1] = new LayerPooling(2, "Pooling", "Max", bufferSize[1], depth2, sizeFM2, sizeK2, stride2, padding2, true);
    layer_test[2] = new LayerConv(3, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight3, numK3, lutNum, af, bufferSize[2], depth3, sizeFM3, sizeK3, stride3, padding3);
    layer_test[3] = new LayerConv(4, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight4, numK4, lutNum, af, bufferSize[3], depth4, sizeFM4, sizeK4, stride4, padding4);
    layer_test[4] = new LayerConv(5, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight5, numK5, lutNum, af, bufferSize[4], depth5, sizeFM5, sizeK5, stride5, padding5);
    layer_test[5] = new LayerConv(6, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight6, numK6, lutNum, af, bufferSize[5], depth6, sizeFM6, sizeK6, stride6, padding6);
    layer_test[6] = new LayerConv(7, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight7, numK7, lutNum, af, bufferSize[6], depth7, sizeFM7, sizeK7, stride7, padding7, true);
    layer_test[7] = new LayerConv(8, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight8, numK8, lutNum, af, bufferSize[7], depth8, sizeFM8, sizeK8, stride8, padding8);
    layer_test[8] = new LayerConv(9, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight9, numK9, lutNum, af, bufferSize[8], depth9, sizeFM9, sizeK9, stride9, padding9);
    layer_test[9] = new LayerConv(10, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight10, numK10, lutNum, af, bufferSize[9], depth10, sizeFM10, sizeK10, stride10, padding10);
    layer_test[10] = new LayerConv(11, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight11, numK11, lutNum, af, bufferSize[10], depth11, sizeFM11, sizeK11, stride11, padding11, true);
    layer_test[11] = new LayerConv(12, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight12, numK12, lutNum, af, bufferSize[11], depth12, sizeFM12, sizeK12, stride12, padding12);
    layer_test[12] = new LayerConv(13, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight13, numK13, lutNum, af, bufferSize[12], depth13, sizeFM13, sizeK13, stride13, padding13);
    layer_test[13] = new LayerConv(14, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight14, numK14, lutNum, af, bufferSize[13], depth14, sizeFM14, sizeK14, stride14, padding14);
    layer_test[14] = new LayerConv(15, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight15, numK15, lutNum, af, bufferSize[14], depth15, sizeFM15, sizeK15, stride15, padding15, true);
    layer_test[15] = new LayerConv(16, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight16, numK16, lutNum, af, bufferSize[15], depth16, sizeFM16, sizeK16, stride16, padding16);
    layer_test[16] = new LayerConv(17, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight17, numK17, lutNum, af, bufferSize[16], depth17, sizeFM17, sizeK17, stride17, padding17);
    layer_test[17] = new LayerConv(18, "Conv", devicePrecision, arraySizeX, arraySizeY, numADC, weight18, numK18, lutNum, af, bufferSize[17], depth18, sizeFM18, sizeK18, stride18, padding18);
    layer_test[18] = new LayerPooling(19, "Pooling", "Max", bufferSize[18], depth19, sizeFM19, sizeK19, stride19, padding19);
    layer_test[19] = new LayerFC(20, "FC", devicePrecision, arraySizeX, arraySizeY, numADC, weight20, outNum20, lutNum, af);

    testCode(debug, layerNum, layer_test, sizeFM1, depth1, inputNum);
    
}