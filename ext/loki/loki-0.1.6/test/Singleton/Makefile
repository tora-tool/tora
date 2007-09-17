include ../Makefile.common

BIN1 := Dependencies$(BIN_SUFFIX)
SRC1 := Dependencies.cpp
OBJ1 := $(SRC1:.cpp=.o)
BIN2 := Phoenix$(BIN_SUFFIX)
SRC2 := Phoenix.cpp
OBJ2 := $(SRC2:.cpp=.o)

.PHONY: all clean
all: $(BIN1) $(BIN2)
clean: cleandeps
	$(RM) $(BIN1)
	$(RM) $(OBJ1)
	$(RM) $(BIN2)
	$(RM) $(OBJ2)

$(BIN1): $(OBJ1)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BIN2): $(OBJ2)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

include ../../Makefile.deps
