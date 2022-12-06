CXX = g++ 
CXXFLAGS = -g -Wall -std=c++17

TARGET = test_program

BUILD_DIR = ./build

//SRCS = $(wildcard *.cpp)
//SRCS = buffer.cpp buffer_test.cpp
//SRCS = tile.cpp tile_test.cpp
//SRCS = lut.cpp lut_test.cpp
SRCS = buffer.cpp tile.cpp lut.cpp layer.cpp layer_test.cpp
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

OUTPUTFILE = program.out

all: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET) | tee $(BUILD_DIR)/$(OUTPUTFILE)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@
 
$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: clean
.PHONY: all

clean:
	-rm -r $(BUILD_DIR)

-include $(DEPS)
