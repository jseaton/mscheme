%{
#include <stdio.h>
#include "mscheme.h"
  
  extern YY_FLUSH_BUFFER;
  extern FILE *yyin;

  void yyerror(const char *str, struct s_environ *env) {
    fprintf(stderr,"error: %s\n",str);
  }
  
  int yywrap() {
    yyin = stdin;
    printf(">>>> ");
    return 0;
  }
  
  int main(int argc, char *argv[]) {
    s_environ *env = global_env();
    FILE *f = fopen("lib", "r");
    if (!f) {
      printf("Cannot open library!");
      return -1;
    }
    
    yyin = f;
    yyparse(env);
    
    return 0;
  }

#define YYSTYPE s_value*

  %}

%parse-param {s_environ *env}

%token LPAREN RPAREN DOT QUOTE BACKTICK COMMA ATCOMMA T_ATOM T_INTG T_BOOL

%%

program: sexpr { s_value *t = eval($1,env); if (yyin == stdin) { printf("=> "); print(t); printf(">>>> ");}}
   | program sexpr { s_value *t = eval($2,env); if (yyin == stdin) { printf("=> "); print(t); printf(">>>> ");}};

list: LPAREN members RPAREN       {$$ = $2;}
   | LPAREN RPAREN                 {$$ = mkcons(0,0);}
   ;
   
sexpr: T_ATOM
   | T_INTG
   | T_BOOL
   | QUOTE sexpr    {$$ = mkcons(mkatom("quote"),mkcons($2,0)); }
   | BACKTICK sexpr {$$ = mkcons(mkatom("quasiquote"),mkcons($2,0)); }
   | COMMA sexpr    {$$ = mkcons(mkatom("unquote"),mkcons($2,0)); }
   | ATCOMMA sexpr  {$$ = mkcons(mkatom("unquote-splicing"),mkcons($2,0)); }
   | list
   ;

members: DOT sexpr {$$ = $2;}
   | sexpr         {$$ = mkcons($1,0);}
   | sexpr members {$$ = mkcons($1,$2);}
   ;
