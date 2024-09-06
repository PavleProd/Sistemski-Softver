all: lexer.cpp parser.cpp

parser.cpp: parser.y lexer.l
	bison -d $^

lexer.cpp: lexer.l
	flex $^

clean:
	rm -rf lexer.hpp lexer.cpp parser.hpp parser.cpp