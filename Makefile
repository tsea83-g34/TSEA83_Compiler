CCX=g++-7
CCXFLAGS = -std=c++17 -c -g
LDFLAGS = -g -o compiler -std=c++17 

compiler: main.o lexer.o
	$(CCX) $(LDFLAGS) main.o lexer.o

main.o: src/main.cpp include/tokens.h
	$(CCX) $(CCXFLAGS) src/main.cpp

lexer.o: include/lexer.h src/lexer.cpp
	$(CCX) $(CCXFLAGS) src/lexer.cpp

clean:
	rm compiler main.o lexer.o
