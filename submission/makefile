.PHONY: all clean

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -o hw5 *.c *.cpp

clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5

zip:
	rm -f 205743230-315479808.zip 	
	zip -r 205743230-315479808.zip *.h *.hpp *.cpp parser.ypp scanner.lex
	./selfcheck-hw5 205743230-315479808.zip 

test:
	cp hw5 ../hw5
	cd ../tests && ./run_tests
	cd ../tests/external_tests && ./run.sh
	cd ../tests/external_tests2 && ./run.sh
	cd ../tests/external_tests3 && python3 compile_and_run.py


semantical:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -o ../tests/semantical_tests/hw3 *.c *.cpp
	cd ../tests/semantical_tests && ./run_semantical_tests