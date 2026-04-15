ifeq ($(OS), Windows_NT)
    TARGET  = main.exe
    RUN     = ./main.exe
else
    TARGET  = main
    RUN     = ./main
endif

ifeq ($(origin CXX), default)
    CXX = clang++
endif
ifeq ($(CXX),)
    CXX = clang++
endif

CXXFLAGS ?= -std=c++20

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

run: $(TARGET)
	$(RUN)

clean:
	rm -f $(TARGET)
