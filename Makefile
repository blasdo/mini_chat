SRCS = connectionSocket.cpp main.cpp
CXXFLAGS = -std=c++98 -g

all:
	clang++ $(CXXFLAGS) $(SRCS);
