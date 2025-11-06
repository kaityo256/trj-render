TARGET=trj2png
CPP := $(shell ls *.cpp external/lodepng/*.cpp)
OBJ := $(patsubst %.cpp,%.o,$(CPP))
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iexternal/lodepng -Iexternal/lammpstrj-parser/include -Iexternal/param

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean dep

clean:
	rm -f $(OBJ) $(TARGET)

dep:
	g++ -MM $(CPP) $(CXXFLAGS) > makefile.dep

-include makefile.dep
