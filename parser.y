%{
  #include <stdio.h>
  #include "mscheme.h"

  void yyerror(const char *str, struct s_environ *env) {
    fprintf(stderr,"error: %s\n",str);
  }
 
  int yywrap() {
    return 1;
  }

  //int main() {
  //yyparse();
  //} 
#define YYSTYPE s_value*

  %}

%parse-param {s_environ *env}

%token LPAREN RPAREN QUOTE BACKTICK COMMA ATCOMMA T_ATOM T_INTG T_BOOL

%%

program: sexpr { s_value *t = b_eval($1,env); printf("=> "); print(t); }
   | program sexpr { s_value *t = b_eval($1,env); printf("=> "); print(t); };

 list: LPAREN members RPAREN       {$$ = $2;}
   | LPAREN RPAREN                 {$$ = mkcons(0,0);}
   ;
   
 sexpr: T_ATOM
   | T_INTG
   | T_BOOL
   | QUOTE sexpr {$$ = mkcons(mkatom("quote"),mkcons($2,0)); }
   | BACKTICK sexpr {$$ = mkcons(mkatom("quasiquote"),mkcons($2,0)); }
   | COMMA sexpr {$$ = mkcons(mkatom("unquote"),mkcons($2,0)); }
   | ATCOMMA sexpr {$$ = mkcons(mkatom("unquote-splicing"),mkcons($2,0)); }
   | list
   ;
 members: sexpr              {$$ = mkcons($1,0);}
   | sexpr members           {$$ = mkcons($1,$2);}
   ;
