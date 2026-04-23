.PHONY: all build format run clean help

LLVM_CONFIG  := ../vendor/llvm-lldb-clang-lld-20.1.7-Release-Dynamic/bin/llvm-config
LLVM_INCLUDE := -I$(shell $(LLVM_CONFIG) --includedir)
LLVM_LIBS    := $(shell $(LLVM_CONFIG) --libs core support native passes irreader)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_SYSLIBS := $(shell $(LLVM_CONFIG) --system-libs)
LLVM_LIBDIR  := $(shell $(LLVM_CONFIG) --libdir)

ifeq ($(origin CXX), default)
    CXX = clang++
endif
ifeq ($(CXX),)
    CXX = clang++
endif

CLANG_FORMAT := ../vendor/llvm-lldb-clang-lld-20.1.7-Release-Dynamic/bin/clang-format

ifeq ($(OS), Windows_NT)
    TARGET     = jit-cc.exe
    RUN        = jit-cc.exe
    FORMAT_CMD = for /r . %%f in (*.cpp *.h *.hpp) do \
                     @if exist "%%f" $(CLANG_FORMAT) -i "%%f"
    CPP_FILES  := main.cpp
else
    TARGET     = jit-cc
    RUN        = ./jit-cc
    CPP_FILES  := $(shell find . -type f -name "*.cpp" ! -path "./build/*")
    HDR_FILES  := $(shell find . -type f \( -name "*.h" -o -name "*.hpp" \) \
                      ! -path "./build/*")
    FORMAT_CMD  = for file in $(CPP_FILES) $(HDR_FILES); do \
                      [ -f "$$file" ] && $(CLANG_FORMAT) -i "$$file"; \
                  done
endif

CXXFLAGS ?= -std=c++23 \
            -I./frontend \
            -I./middle   \
            -I./backend  \
            $(LLVM_INCLUDE)

LDFLAGS := $(LLVM_LDFLAGS) \
           -Wl,-rpath,$(LLVM_LIBDIR) \
           $(LLVM_LIBS) \
           $(LLVM_SYSLIBS)

all: format build

build:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(CPP_FILES) $(LDFLAGS)

format:
	@echo "Formatting source files..."
ifeq ($(OS), Windows_NT)
	@$(FORMAT_CMD)
else
	@if command -v $(CLANG_FORMAT) >/dev/null 2>&1; then \
		$(FORMAT_CMD); \
	else \
		echo "clang-format not found (looked for $(CLANG_FORMAT))"; \
	fi
endif
	@echo "Formatting complete."

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
	@cmd /c echo   make all      : Format then build  (default)
	@cmd /c echo   make help     : Show this help
else
	@echo "Usage:"
	@echo ""
	@echo "  make build     - Compile the project"
	@echo "  make format    - Format all .cpp / .h / .hpp files"
	@echo "  make run       - Build and run the executable"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make all       - Format then build (default)"
	@echo "  make help      - Show this help"
endif
