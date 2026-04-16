.PHONY: all build format run clean help

ifeq ($(OS), Windows_NT)
    TARGET  = jit-cc.exe
    RUN     = jit-cc.exe

    FORMAT_CMD = for /r . %%f in (*.cpp *.h) do @if exist "%%f" clang-format -i "%%f"

    CPP_FILES := main.cpp
else
    TARGET  = jit-cc
    RUN     = ./jit-cc

    CPP_FILES := $(shell find . -type f -name "*.cpp" ! -path "./build/*")
    H_FILES   := $(shell find . -type f -name "*.h" ! -path "./build/*")

    FORMAT_CMD = for file in $$(find . -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "./build/*"); do \
                    [ -f "$$file" ] && clang-format -i "$$file"; \
                 done
endif

ifeq ($(origin CXX), default)
    CXX = clang++
endif
ifeq ($(CXX),)
    CXX = clang++
endif

CXXFLAGS ?= -std=c++20 -I./frontend

all: format build

build:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(CPP_FILES)

format:
	@echo Formatting source files...
ifeq ($(OS), Windows_NT)
	@$(FORMAT_CMD)
else
	@if command -v clang-format >/dev/null 2>&1; then \
		$(FORMAT_CMD); \
	else \
		echo "clang-format not found"; \
	fi
endif
	@echo Formatting complete

run: build
	$(RUN)

clean:
ifeq ($(OS), Windows_NT)
	del /q $(TARGET) 2>nul
else
	rm -f $(TARGET)
endif

help:
ifeq ($(OS), Windows_NT)
	@cmd /c echo Usage:
	@cmd /c echo.
	@cmd /c echo   make build    : Compile the project
	@cmd /c echo   make format   : Format all source files with clang-format
	@cmd /c echo   make run      : Build and run the executable
	@cmd /c echo   make clean    : Remove build artifacts
	@cmd /c echo   make all      : Format and build (default)
	@cmd /c echo   make help     : Show this help message
else
	@echo "Usage:"
	@echo ""
	@echo "  make build     - Compile the project"
	@echo "  make format    - Format all source files with clang-format"
	@echo "  make run       - Build and run the executable"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make all       - Format and build (default)"
	@echo "  make help      - Show this help message"
endif
