all:	Avm.h Avm.cpp Instructions.h Assign.h Arithmetic.h Functions.h Relational.h Table.h help.h
	g++ -std=c++11 -o avm Avm.cpp Assign.cpp Arithmetic.cpp Functions.cpp Relational.cpp Table.cpp help.cpp

help:	Avm.h Avm.cpp Instructions.h Assign.h Arithmetic.h Functions.h Relational.h Table.h help.h
	g++ -g -std=c++11 -o avm Avm.cpp Assign.cpp Arithmetic.cpp Functions.cpp Relational.cpp Table.cpp help.cpp

run:	avm instructions.abc	
	gdb --args avm instructions.abc

clean:
	rm -rf avm
	rm -rf help.txt
	rm -rf instructions.abc
