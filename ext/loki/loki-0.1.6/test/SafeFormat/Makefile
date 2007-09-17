include ../Makefile.common

BIN := main$(BIN_SUFFIX)
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)
CXXFLAGS := $(CXXWARNFLAGS) -g -fexpensive-optimizations -O3

.PHONY: all clean
all: $(BIN)
clean: cleandeps
	$(RM) $(BIN)
	$(RM) $(OBJ)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

include ../../Makefile.deps
