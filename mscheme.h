#include <glib.h>
#include "debug.h"

typedef enum s_type {ATOM,CONS,PROC,BLTN,TAIL,BOOL,INTG,TRAN} s_type;

typedef union value s_value;
typedef struct environ s_environ;

typedef struct atom {
  s_type type;
  char   *val;
} s_atom;

typedef struct cons {
  s_type   type;
  s_value *curr;
  s_value *next;
} s_cons;

typedef struct proc {
  s_type    type;
  s_value  *body;
  s_value  *args;
  s_environ *env;
} s_proc;

typedef struct bltn {
  s_type    type;
  s_value   *(*fun)(s_value *, s_environ *);
  s_environ *env;
} s_bltn;

typedef struct sbln {
  s_type type;
  char   val;
} s_bool;

typedef struct intg {
  s_type type;
  int    val;
} s_intg;

typedef struct tran {
  s_type type;
  s_environ *lits;
  s_value *rules;
} s_tran;

typedef union value {
  s_type type;
  s_atom atom;
  s_cons cons;
  s_proc proc;
  s_bltn bltn;
  s_bltn tail;
  s_bool sbln;
  s_intg intg;
  s_tran tran;
} s_value;

struct environ {
  GHashTable *contents;
  s_environ  *parent;
};	

s_value *eval(s_value *prog, s_environ *env);

s_value *mkatom(char *s);
s_atom rtatom(s_value *p);

s_value *mkcons(s_value *p, s_value *n);

s_value *mkbltn(s_value *(*fun)(s_value *, s_environ *));
s_bltn  rtbltn(s_value *p);

s_value *mktail(s_value *(*fun)(s_value *, s_environ *));
s_bltn  rttail(s_value *p);

s_value *mkbool(char b);
s_bool  rtbool(s_value *p);

s_value *mkintg(int i);
s_intg  rtintg(s_value *p);

s_value *lambda(s_value *args, s_value *body, s_environ *env);
s_proc  rtproc(s_value *p);

s_value *car(s_value *p);

s_value *cdr(s_value *p);

void set_car(s_value * p, s_value *s);

void set_cdr(s_value * p, s_value *s);

s_environ *env_new(s_environ *parent);

void env_add(s_environ *env, char *key, s_value *value);

s_value *env_get(s_environ *env, char *key);

void env_set(s_environ *env, char *key, s_value *value);

void print(s_value *args);

void print_env(s_environ *env);

s_environ *global_env();

s_value *eval(s_value *prog, s_environ *env);

void dprint(s_value *args);
void dprint_env(s_environ *env);
