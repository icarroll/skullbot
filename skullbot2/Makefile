tokenize: lex.yy.cc
	g++ -std=gnu++11 lex.yy.cc -I/usr/include -o tokenize.exe

lex.yy.cc: tokenize.l
	flex++ tokenize.l
