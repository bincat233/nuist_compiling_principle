mycc: lexical_analysis.c parser.cpp
	gcc -c lexical_analysis.c -std=c99
	g++ parser.cpp lexical_analysis.o -o mycc

regextest: regex_test.c
	gcc -o regex_test regex_test.c -std=c99

clean:
	rm *.o
