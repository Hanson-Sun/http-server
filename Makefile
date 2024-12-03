CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -I./

SRCS1 = main.cpp HTTPServer.cpp
SRCS2 = ./tests/test.cpp HTTPServer.cpp

OBJS1 = $(SRCS1:.cpp=.o)
OBJS2 = $(SRCS2:.cpp=.o)

TARGETS = main test

all: $(TARGETS)

main: $(OBJS1)
	$(CXX) $(CXXFLAGS) -o main $(OBJS1)

test: $(OBJS2)
	$(CXX) $(CXXFLAGS) -o tests/test $(OBJS2)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS1) $(OBJS2) $(TARGETS)

.PHONY: all clean