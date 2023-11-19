CXX = g++
CXXFLAGS = -std=c++17

all: islands-1-rec islands-2-neib-queue

islands-1-rec: islands-1-rec.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

islands-2-neib-queue: islands-2-neib-queue.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f islands-1-rec islands-2-neib-queue
