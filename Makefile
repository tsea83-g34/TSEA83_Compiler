CCX=g++-7
CCXFLAGS = -std=c++17 -g

HEADERS = $(wildcard include/*.h)
SOURCES = $(wildcard src/*.cpp)
OBJS	= $(patsubst *.cpp, %.o, $(SOURCES))

compiler: $(OBJS)
	$(CCX) $(CCXFLAGS) $(OBJS) -o compiler

obj/%.o: src/%.cpp $(HEADERS)
	$(CCX) $(CCXFLAGS) -c $< -o $@

clean:
	rm -f compiler
