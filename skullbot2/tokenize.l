%{
#include <iostream>
using namespace std;
%}

%option C++ case-insensitive noyywrap ecs meta-ecs

ws [ \t\n\r]

let [a-z]
sq \'
dq \"

abbrev (mr|mrs|st|esq|edw|{let})\.

word {let}({sq}?{let})*(s{sq})?|{abbrev}

%%

{ws}   /* skip */

{word}   cout << "word get" << endl;

{sq}   cout << "sq get" << endl;

%%

int main(int nargs, char * args[]) {
    FlexLexer * lexer = new yyFlexLexer;
    while (lexer->yylex() != 0) {}

    return 0;
}
