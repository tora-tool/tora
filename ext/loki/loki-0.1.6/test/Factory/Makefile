include ../Makefile.common

BIN := Factory$(BIN_SUFFIX)
SRC := Factory.cpp
OBJ := $(SRC:.cpp=.o)

.PHONY: all clean
all: $(BIN)
clean: cleandeps
	$(RM) $(BIN)
	$(RM) $(OBJ)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

include ../../Makefile.deps
