CCX=g++-7
CCXFLAGS = -std=c++17 -c -g
LDFLAGS = -g -o compiler -std=c++17 

compiler: main.o lexer.o parser.o parser_types.o
	$(CCX) $(LDFLAGS) main.o lexer.o parser.o

main.o: src/main.cpp include/tokens.h include/parser_types.h include/parser.h
	$(CCX) $(CCXFLAGS) src/main.cpp

lexer.o: include/lexer.h src/lexer.cpp
	$(CCX) $(CCXFLAGS) src/lexer.cpp

parser.o: include/parser.h include/parser_types.h src/parser.cpp
	$(CCX) $(CCXFLAGS) src/parser.cpp

parser_types.o: include/parser_types.h include/parser.h src/parser_types.cpp
	$(CCX) $(CCXFLAGS) src/parser_types.cpp

clean:
	rm compiler main.o lexer.o parser.o parser_types.o interfaces.o
