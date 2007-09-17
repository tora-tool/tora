include ../Makefile.common

ifeq ($(OS),Darwin)
  LIB1 := libfoo.dylib
  LIB2 := libsingletondll.dylib
  SHARED_LIB_FLAG := -dynamiclib -fPIC
else
ifeq ($(OS),Linux)
  LIB1 := libfoo.so
  LIB2 := libsingletondll.so
  SHARED_LIB_FLAG := --shared -fPIC
else
  LIB1 := foo.dll
  LIB2 := singletondll.dll
  SHARED_LIB_FLAG := --shared
endif
endif

BIN1 := client$(BIN_SUFFIX)
SRC1 := client.cpp
OBJ1 := $(SRC1:.cpp=.o)
SRC2 := foo.cpp
OBJ2 := $(SRC2:.cpp=.lo)
SRC3 := singletondll.cpp
OBJ3 := $(SRC3:.cpp=.lo)
override LDFLAGS := -L../../lib

.PHONY: all clean
all: $(BIN1)

%.lo : %.cpp
	$(CXX) -c $(CXXFLAGS) -fPIC $(CPPFLAGS) -o $@ $<

$(BIN1): $(OBJ1) $(LIB1) $(LIB2)
	$(CXX) -L. $(LDFLAGS) -o $@ $(OBJ1) $(LDLIBS) -lfoo -lsingletondll

$(LIB1): $(OBJ2)
	$(CXX) $(SHARED_LIB_FLAG) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(LIB2): $(OBJ3)
	$(CXX) $(SHARED_LIB_FLAG) $(LDFLAGS) -L. -o $@ $^ -lfoo $(LDLIBS)

clean: cleandeps
	$(RM) $(BIN1)
	$(RM) $(OBJ1)
	$(RM) $(LIB1)
	$(RM) $(OBJ2)
	$(RM) $(LIB2)
	$(RM) $(OBJ3)

include ../../Makefile.deps
