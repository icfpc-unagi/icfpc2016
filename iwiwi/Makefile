CXX = g++
CXXFLAGS = -O2 -std=c++0x -Wall -Wextra -lgmp -lgflags

all: bin bin/prefilter bin/postfilter1 bin/postfilter2 bin/score bin/perimeter_detector

bin:
	mkdir -p bin

bin/prefilter: prefilter.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/postfilter1: postfilter1.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/postfilter2: postfilter2.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/score: score.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

bin/perimeter_detector: perimeter_detector.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf bin
