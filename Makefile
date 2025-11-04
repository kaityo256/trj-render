TARGET=trj2png
CPP=$(shell ls *.cpp external/lodepng/*.cpp)
CXX = g++
CXXFLAGS = -std=c++14 -O2 -Iexternal/lodepng -Iexternal/lammpstrj/include -Iexternal/param

all: $(TARGET)

trj2png:
	$(CXX) $(CXXFLAGS) main.cpp external/lodepng/lodepng.cpp -o $@

.PHONY: clean dep

clean:
	rm -f $(TARGET)

dep:
	g++ -MM $(CPP) $(CXXFLAGS) > makefile.dep

-include makefile.dep