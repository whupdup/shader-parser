
CXXFLAGS := -std=c++17 -I$(CURDIR)

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d)) 

SRC_FILES := $(call rwildcard, ./, *.cpp)
OBJ_FILES := $(SRC_FILES:%=bin/%.o)

all: shader-parser

run:
	./shader-parser test-shader.glsl

shader-parser: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all run