
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

TARGET = pagingwithatc

SRCS = main.cpp PageTable.cpp TLB.cpp log.cpp tracereader.cpp

HEADERS = PageTable.h TLB.h log.h tracereader.h

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

