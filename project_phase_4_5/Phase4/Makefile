all: Parser.y Scanner.l SymbolTable.h SymbolTable.cpp Quad.h Quad.cpp Instructions.h Instructions.cpp
	bison -v --yacc --defines --output=Parser.cpp Parser.y
	flex --outfile=al.cpp Scanner.l
	g++ -std=c++11 -o alpha Parser.cpp al.cpp SymbolTable.cpp Quad.cpp Instructions.cpp

help: Parser.y Scanner.l SymbolTable.h SymbolTable.cpp Quad.h Quad.cpp Instructions.h Instructions.cpp
	bison -v --yacc --defines --output=Parser.cpp Parser.y
	flex --outfile=al.cpp Scanner.l
	g++ -g -std=c++11 -o alpha Parser.cpp al.cpp SymbolTable.cpp Quad.cpp Instructions.cpp

clean:
	rm al.cpp
	rm al.h
	rm Parser.output
	rm Parser.hpp
	rm Parser.cpp
	rm alpha
	rm syntax.txt
	rm symbolTable.txt
	rm quads.txt
	rm instructions.txt
