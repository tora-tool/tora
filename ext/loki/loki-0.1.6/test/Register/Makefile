include ../Makefile.common

BIN1 := main$(BIN_SUFFIX)
SRC1 := main.cpp
OBJ1 := $(SRC1:.cpp=.o)
LIB1 := libfoo.a
SRC2 := foo.cpp
OBJ2 := $(SRC2:.cpp=.o)
CPPFLAGS += -DLOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT

.PHONY: all clean
all: $(BIN1)

$(BIN1): $(OBJ1) $(LIB1) 
	$(CXX) $(CXXFLAGS) -L. $(LDFLAGS) $(CPPFLAGS) -o $@ $(OBJ1) $(LDLIBS) -lfoo

$(LIB1): $(OBJ2)
	$(AR) $(ARFLAGS) $@ $^

clean: cleandeps
	$(RM) $(BIN1)
	$(RM) $(OBJ1)
	$(RM) $(LIB1)
	$(RM) $(OBJ2)

include ../../Makefile.deps
