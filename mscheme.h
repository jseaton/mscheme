#include <glib.h>

typedef enum s_type {ATOM,CONS,PROC,BLTN,QUOT,BOOL,INTG,TRAN} s_type;

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

s_value *mkcons(s_value *p, s_value *n);

s_value *mkbltn(s_value *(*fun)(s_value *, s_environ *));

s_value *mkbool(char b);

s_value *mkintg(int i);

s_value *lambda(s_value *args, s_value *body, s_environ *env);

s_value *car(s_value *p);

s_value *cdr(s_value *p);

s_environ *env_new(s_environ *parent);

void env_add(s_environ *env, char *key, s_value *value);

s_value *env_get(s_environ *env, char *key);

void env_set(s_environ *env, char *key, s_value *value);

void print(s_value *args);

s_environ *global_env();

s_value *b_eval(s_value *prog, s_environ *env);
