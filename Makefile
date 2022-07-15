# ======================================================================
#  Hotel Management System - Makefile
# ----------------------------------------------------------------------
#  Usage:
#     make          Build the executable
#     make run      Build (if needed) and run the program
#     make clean    Remove the compiled binary and artefacts
# ======================================================================

CXX      := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -O2
TARGET   := hotel
SRCDIR   := src
SRC      := $(SRCDIR)/main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
	@echo "Build complete -> ./$(TARGET)"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
	@echo "Cleaned build artefacts."
