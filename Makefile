all: a.out

a.out:
	g++ main.cpp lodepng.cpp

.PHONY: clean

clean:
	rm -f a.out
