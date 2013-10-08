.PRECIOUS=.o

#ANTLRGRAMMARS ?= $(wildcard *.g)
ANTLRGRAMMARS = PLSQLGuiLexer.g PLSQLGuiLexerJava.g MySQLGuiLexer.g #PLSQLParser.g
HEADERS = $(wildcard *.hpp) $(wildcard *.inl)
SOURCES = $(wildcard *.cpp)
PSOURCES = $(wildcard PLSQLParser*.cpp) PLSQLLexer.cpp
POBJS = $(PSOURCES:.cpp=.o)
GRAMMAROPTIONS= -report -Xconversiontimeout 1500000 -Xmultithreaded -Xwatchconversion
ANTLR = antlr-complete-3.5.2-SNAPSHOT.jar

.SUFFIXES:

INCLUDES= -I. -I../libantlr3cpp-3.5.1/include/ -I../../../boost_1_46_1/

CXXFLAGS=-ggdb3 -O0 -fno-inline -Wno-parentheses-equality
##CXXFLAGS=-ggdb3 -Os

TOBJS= utils.o

all: t0 t1 t2 t4 m0 p0 s0 s1 s2

$(ANTLR): org/antlr/codegen/templates/Cpp/Cpp.stg \
	org/antlr/codegen/templates/Cpp/AST.stg \
	org/antlr/codegen/templates/Cpp/ASTParser.stg \
	org/antlr/codegen/templates/Java/Java.stg
	jar uvf $(ANTLR) org/antlr/codegen/templates/*/*.stg

t0:	t0.cpp PLSQLGuiLexer.tokens t0.cpp $(TOBJS) PLSQLGuiLexer.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) t0.cpp PLSQLGuiLexer.o $(LIBS) -o $@

t1:	t1.cpp PLSQLLexer.tokens PLSQLParser.tokens t1.cpp $(TOBJS) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) t1.cpp $(POBJS) -o $@

t2:	t2.cpp PLSQLLexer.tokens PLSQLParser.tokens t1.cpp $(TOBJS) $(PSOURCES) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSEBOOST $(INCLUDES) $(TOBJS) t2.cpp $(POBJS) $(LIBS) -o $@

t4:	t4.cpp PLSQLLexer.tokens PLSQLParser.tokens t4.cpp $(TOBJS) $(PSOURCES) $(POBJS) Makefile UserTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) t4.cpp $(POBJS) -o $@

m0:	m0.cpp MySQLGuiLexer.tokens $(TOBJS) MySQLGuiLexer.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) m0.cpp MySQLGuiLexer.o $(LIBS) -o $@

p0:	p0.cpp OraclePLSQL.tokens $(TOBJS) OraclePLSQLLexer.o OraclePLSQLParser.o Makefile UserGuiTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) p0.cpp OraclePLSQLLexer.o OraclePLSQLParser.o -o $@

s0:	s0.cpp SQLMini.tokens $(TOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) s0.cpp SQLMiniLexer.cpp SQLMiniParser.cpp $(LIBS) -o $@

s1:	s1.cpp S1.tokens $(TOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) s1.cpp S1Lexer.cpp S1Parser.cpp $(LIBS) -o $@

s2:	s2.cpp S2.tokens $(TOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) s2.cpp S2Lexer.cpp S2Parser.cpp $(LIBS) -o $@

s3:	s3.cpp S3.tokens $(TOBJS) Makefile UserMiniTraits.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) $(TOBJS) s3.cpp S3Lexer.cpp S3Parser.cpp $(LIBS) -o $@

clean:
	rm -f *.o *Lexer*.[ch]pp *Parser.[ch]pp t0 t1 t2 t3 t4 m0 s0 s1 s2 *.tokens

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
	cp PLSQLParser.hpp   ../../src/parsing/PLSQLParser.hpp
	cp PLSQLParser_PLSQLCommons.hpp    ../../src/parsing/PLSQLParser_PLSQLCommons.hpp
	cp PLSQLParser_PLSQL_DMLParser.hpp ../../src/parsing/PLSQLParser_PLSQL_DMLParser.hpp
	cp PLSQLParser_PLSQLKeys.hpp       ../../src/parsing/PLSQLParser_PLSQLKeys.hpp
	cp PLSQLParser_SQLPLUSParser.hpp   ../../src/parsing/PLSQLParser_SQLPLUSParser.hpp

%.u: %.g
	@echo "Bulding dependencies for "$<
	java -jar $(ANTLR) -depend $< > $@
	@grep ":" $@ |awk 'BEGIN {printf "ANTLRGENLIST := " }{printf " " $$1}END {print ""}'  >> $@.tmp
	@cat $@.tmp >> $@
	$(RM) $@.tmp

%.tokens %.cpp %Lexer.c %Parser.c %Lexer.h %Parser.h %.hpp: %.g $(ANTLR)
	java -Xmx1024m -jar $(ANTLR) $(GRAMMAROPTIONS) $<

# PLSQL_DMLParser.o: PLSQL_DMLParser.cpp PLSQL_DMLParser.tokens
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -c PLSQL_DMLParser.c -o $@


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ANTLRGRAMMARS)),)
#-include $(ANTLRGRAMMARS:.g=.u)
endif
endif

%.o: %.cpp $(HEADERS) utils.hpp
	$(CXX) $(CXXFLAGS) -DUSESTL $(INCLUDES) -c $< -o $@
