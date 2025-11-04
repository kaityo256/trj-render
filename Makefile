TARGET=trj2png
CXX = g++
CXXFLAGS = -std=c++14 -O2 -Iexternal/lammpstrj/include -Iexternal/param

all: $(TARGET)

trj2png: main.cpp canvas.hpp lodepng.cpp lodepng.h
	$(CXX) $(CXXFLAGS) main.cpp lodepng.cpp -o $@

.PHONY: clean

clean:
	rm -f $(TARGET)
