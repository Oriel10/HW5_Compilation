.PHONY: all clean

all: clean
	flex scanner.lex
	bison -d parser.ypp
	g++ -std=c++17 -o hw5 *.c *.cpp
clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5

test:
	cd tests && ./run_tests

semantical:
	flex scanner.lex
	bison -d parser.ypp
	g++ -std=c++17 -o tests/semantical_tests/hw3 *.c *.cpp
	cd tests/semantical_tests && ./run_semantical_tests