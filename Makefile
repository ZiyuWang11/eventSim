CXX = g++
CXXFLAGS = -g -Wall -std=c++17

TARGET = test_program

BUILD_DIR = ./build

//SRCS = $(wildcard *.cpp)
SRCS = tile.cpp tile_test.cpp
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
