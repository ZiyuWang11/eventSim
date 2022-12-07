// layerabc.cpp - Abstract Layer Methods

#include <string>
#include "layerabc.h"

LayerABC::LayerABC(int layerNum, const std::string layerType)
{
    layerNum_ = layerNum;
    layerType_ = layerType;
}
