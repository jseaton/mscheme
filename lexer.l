%{
#include <stdio.h>
#include "mscheme.h"
#define YYSTYPE s_value*

#include "parser.tab.h"
%}

%%

\( { return LPAREN; }
\) { return RPAREN; }
#t { yylval=mkbool(1); return T_BOOL; }
#f { yylval=mkbool(0); return T_BOOL; }
\' { return QUOTE; }
\` { return BACKTICK; }
\,@ { return ATCOMMA; }
\, { return COMMA; }
\"(\\.|[^\\"])*\" { yylval=mkatom(strdup(yytext)); return T_ATOM; }
([a-z\!\$\%\&\*\/\:\<\=\>\?\^\_\~][a-z0-9\!\$\%\&\*\/\:\<\=\>\?\^\_\~\+\-\.\@]*)|\+|\-|\.\.\. { yylval=mkatom(strdup(yytext)); return T_ATOM; }
[0-9][0-9]* { yylval=mkintg(atoi(yytext)); return T_INTG; }
[ \t\n]+
\;[^\n]*

%%
