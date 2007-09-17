include ../Makefile.common

BIN := main$(BIN_SUFFIX)
SRC := main.cpp strong.cpp LockTest.cpp
OBJ := $(SRC:.cpp=.o)
LDLIBS += -lpthread

.PHONY: all clean
all: $(BIN)
clean: cleandeps
	$(RM) $(BIN)
	$(RM) $(OBJ)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

include ../../Makefile.deps
