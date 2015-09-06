.PRECIOUS=.o

#ANTLRGRAMMARS ?= $(wildcard *.g)
ANTLRGRAMMARS = PLSQLGuiLexer.g PLSQLGuiLexerJava.g MySQLGuiLexer.g #PLSQLParser.g
HEADERS = $(wildcard *.hpp) $(wildcard *.inl)
SOURCES = $(wildcard *.cpp)
PSOURCES = $(wildcard PLSQLParser*.cpp) PLSQLLexer.cpp
POBJS = $(PSOURCES:.cpp=.o)
T5SOURCES = $(wildcard OracleDML*.cpp)
T5OBJS    = $(T5SOURCES:.cpp=.o)
GRAMMAROPTIONS= -report -Xconversiontimeout 1500000 -Xwatchconversion
ANTLR = antlr-complete-3.5.3-SNAPSHOT.jar
CXX = clang++

.SUFFIXES:

INCLUDES= -I. -I../libantlr3cpp-3.5.3/include/
#INCLUDES= -I. -I../../../antlr3/runtime/Cpp/include/ -I../../../boost_1_46_1/

CXXFLAGS=-ggdb3 -O0 -fno-inline -std=c++14
#CXXFLAGS=-ggdb3 -Os -std=c++14

UOBJS= utils.o

all: t0 t1 t2 t4 t5 m0 m1 p0 s0 s1 s2 s3 p1

$(ANTLR): org/antlr/codegen/templates/Cpp/Cpp.stg \
	org/antlr/codegen/templates/Cpp/AST.stg \
	org/antlr/codegen/templates/Cpp/ASTParser.stg \
	org/antlr/codegen/templates/Java/Java.stg
	jar uvf $(ANTLR) org/antlr/codegen/templates/*/*.stg

t0:	t0.cpp PLSQLGuiLexer.tokens t0.cpp $(UOBJS) PLSQLGuiLexer.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) t0.cpp PLSQLGuiLexer.o $(LIBS) -o $@

t1:	t1.cpp PLSQLLexer.tokens PLSQLParser.tokens t1.cpp $(UOBJS) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) t1.cpp $(POBJS) -o $@

t2:	t2.cpp PLSQLLexer.tokens PLSQLParser.tokens t1.cpp $(UOBJS) $(PSOURCES) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSEBOOST $(INCLUDES) $(UOBJS) t2.cpp $(POBJS) $(LIBS) -o $@

t4:	t4.cpp PLSQLLexer.tokens PLSQLParser.tokens $(UOBJS) $(PSOURCES) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) t4.cpp $(POBJS) -o $@

t5:	t5.cpp OracleDMLLexer.tokens OracleDML.tokens $(UOBJS) $(T5OBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) $(T5OBJS) t5.cpp -o $@

check:	check.cpp OracleDMLLexer.tokens OracleDML.tokens $(UOBJS) $(T5OBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) $(T5OBJS) check.cpp -o $@

m0:	m0.cpp MySQLGuiLexer.tokens $(UOBJS) MySQLGuiLexer.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) m0.cpp MySQLGuiLexer.o $(LIBS) -o $@

m1:	m1.cpp MySQLLexer.tokens $(UOBJS) MySQLLexer.o Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) $@.cpp MySQLLexer.o $(LIBS) -o $@

p0:	p0.cpp OraclePLSQL.tokens $(UOBJS) OraclePLSQLLexer.o OraclePLSQLParser.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) p0.cpp OraclePLSQLLexer.o OraclePLSQLParser.o -o $@

s0:	s0.cpp SQLMini.tokens $(UOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) s0.cpp SQLMiniLexer.cpp SQLMiniParser.cpp $(LIBS) -o $@

s1:	s1.cpp S1.tokens $(UOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) s1.cpp S1Lexer.cpp S1Parser.cpp $(LIBS) -o $@

s2:	s2.cpp S2.tokens $(UOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) s2.cpp S2Lexer.cpp S2Parser.cpp $(LIBS) -o $@

s3:	s3.cpp S3.tokens $(UOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) s3.cpp S3Lexer.cpp S3Parser.cpp $(LIBS) -o $@

p1:	p1.cpp PostgreSQLGuiLexer.tokens $(UOBJS) PostgreSQLGuiLexer.g.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(UOBJS) p1.cpp PostgreSQLGuiLexer.o $(LIBS) -o $@

clean:
	rm -f *.o *Lexer*.[ch]pp *Parser.[ch]pp OracleDML*.[ch]pp t0 t1 t2 t4 t5 m0 m1 s0 s1 s2 s3 p0 p1 check *.tokens

install:
	cp PLSQLGuiLexer.cpp ../../src/parsing/PLSQLGuiLexer.cc
	cp PLSQLGuiLexer.hpp ../../src/parsing/PLSQLGuiLexer.hpp
	cp UserGuiTraits.hpp ../../src/parsing/UserGuiTraits.hpp
	cp TokenAttr.hpp     ../../src/parsing/TokenAttr.hpp
	cp PLSQLLexer.cpp    ../../src/parsing/PLSQLLexer.cc
	cp PLSQLParser.cpp   ../../src/parsing/PLSQLParser.cc
	cp PLSQLParser_PLSQLCommons.cpp    ../../src/parsing/PLSQLParser_PLSQLCommons.cc
	cp PLSQLParser_PLSQL_DMLParser.cpp ../../src/parsing/PLSQLParser_PLSQL_DMLParser.cc
	cp PLSQLParser_PLSQLKeys.cpp       ../../src/parsing/PLSQLParser_PLSQLKeys.cc
	cp PLSQLParser_SQLPLUSParser.cpp   ../../src/parsing/PLSQLParser_SQLPLUSParser.cc
	cp PLSQLParser.hpp                 ../../src/parsing/PLSQLParser.hpp
	cp PLSQLParser_PLSQLCommons.hpp    ../../src/parsing/PLSQLParser_PLSQLCommons.hpp
	cp PLSQLParser_PLSQL_DMLParser.hpp ../../src/parsing/PLSQLParser_PLSQL_DMLParser.hpp
	cp PLSQLParser_PLSQLKeys.hpp       ../../src/parsing/PLSQLParser_PLSQLKeys.hpp
	cp PLSQLParser_SQLPLUSParser.hpp   ../../src/parsing/PLSQLParser_SQLPLUSParser.hpp
	cp MySQLLexer.cpp                  ../../src/parsing/MySQLLexer.cc
	cp MySQLLexer.hpp                  ../../src/parsing/MySQLLexer.hpp
	cp MySQLGuiLexer.cpp               ../../src/parsing/MySQLGuiLexer.cc
	cp MySQLGuiLexer.hpp               ../../src/parsing/MySQLGuiLexer.hpp

%.u: %.g
	@echo "Bulding dependencies for "$<
	java -jar $(ANTLR) -depend $< > $@
	@grep ":" $@ |awk 'BEGIN {printf "ANTLRGENLIST := " }{printf " " $$1}END {print ""}'  >> $@.tmp
	@cat $@.tmp >> $@
	$(RM) $@.tmp

%.tokens %Lexer.cpp %Parser.cpp %Lexer.hpp %Parser.hpp: %.g $(ANTLR)
	java -Xmx10240m -jar $(ANTLR) $(GRAMMAROPTIONS) $<

# PLSQL_DMLParser.o: PLSQL_DMLParser.cpp PLSQL_DMLParser.tokens
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -c PLSQL_DMLParser.c -o $@


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ANTLRGRAMMARS)),)
#-include $(ANTLRGRAMMARS:.g=.u)
endif
endif

%.o: %.cpp $(HEADERS) utils.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) -c $< -o $@
