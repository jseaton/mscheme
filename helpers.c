#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <glib.h>

#include "mscheme.h"

s_environ *env_new(s_environ *parent) {
  s_environ *env = (s_environ *)malloc(sizeof(s_environ));
  env->parent   = parent;
  env->contents = g_hash_table_new(g_str_hash,g_str_equal);
  return env;
}

void env_add(s_environ *env, char *key, s_value *value) {
  g_hash_table_replace(env->contents,key,value);
}

s_value *env_get(s_environ *env, char *key) {
  s_value *r = g_hash_table_lookup(env->contents, key);
  if (r)
    return r;
  if (env->parent)
    return env_get(env->parent, key);
  pmesg("Variable %s not found\n", key);
  return 0;
}

void env_set(s_environ *env, char *key, s_value *value) {
  s_value *r = g_hash_table_lookup(env->contents, key);
  if (r)
    return env_add(env, key, value);
  if (env->parent)
    return env_set(env->parent, key, value);
  pmesg("Variable %s not found\n", key);
}	

s_value *mkatom(char *s) {
  s_value *r   = (s_value *)malloc(sizeof(s_atom));
  r->atom.type = ATOM;
  r->atom.val  = s;
  return r;
}

s_atom rtatom(s_value *p) {
  if (p->type == ATOM)
    return p->atom;
  else {
    printf("Not an atom: ");
    print(p);
  }
}

s_value *mkcons(s_value *p, s_value *n) {
  s_value *r   = (s_value *)malloc(sizeof(s_cons));
  r->cons.type = CONS;
  r->cons.curr = p;
  r->cons.next = n;
  return r;
}

s_value *mkbltn(s_value *(*fun)(s_value *, s_environ *)) {
  s_value *r   = (s_value *)malloc(sizeof(s_bltn));
  r->bltn.type = BLTN;
  r->bltn.fun  = fun;
  return r;
}

s_bltn rtbltn(s_value *p) {
  if (p->type == BLTN)
    return p->bltn;
  else {
    printf("Not a builtin: ");
    print(p);
  }
}

s_value *mktail(s_value *(*fun)(s_value *, s_environ *)) {
  s_value *r   = mkbltn(fun);
  r->bltn.type = TAIL;
  return r;
}

s_bltn rttail(s_value *p) {
  if (p->type == TAIL)
    return p->tail;
  else {
    printf("Not a tail: ");
    print(p);
  }
}

s_value *mkbool(char b) {
  s_value *r   = (s_value *)malloc(sizeof(s_bool));
  r->sbln.type = BOOL;
  r->sbln.val  = b;
  return r;
}

s_bool rtbool(s_value *p) {
  if (p->type == BOOL)
    return p->sbln;
  else {
    printf("Not a bool: ");
    print(p);
  }
}

s_value *mkintg(int i) {
  s_value *r   = (s_value *)malloc(sizeof(s_intg));
  r->intg.type = INTG;
  r->intg.val  = i;
  return r;
}

s_intg rtintg(s_value *p) {
  if (p->type == INTG)
    return p->intg;
  else {
    printf("Not an int: ");
    print(p);
  }
}

s_value *lambda(s_value *body, s_value *args, s_environ *env) {
  s_value *r   = (s_value *)malloc(sizeof(s_proc));
  r->proc.type = PROC;
  r->proc.env  = env;
  r->proc.args = args;
  r->proc.body = body;
  return r;
}

s_proc rtproc(s_value *p) {
  if (p->type == PROC)
    return p->proc;
  else {
    printf("Not a proc: ");
    print(p);
  }
}

s_value *mktran(s_value *lits, s_value *rules) {
  pmesg("Mktran: ");
  dprint(lits);
  dprint(rules);
  s_value *r   = (s_value *)malloc(sizeof(s_tran));
  r->tran.type = TRAN;
  r->tran.lits  = env_new(0);
  r->tran.rules = rules;
  while (lits && car(lits)) {
    env_add(r->tran.lits,car(lits)->atom.val,mkbool(1));
    lits = cdr(lits);
  }
  pmesg("----\n");
  dprint_env(r->tran.lits);
  return r;
}

s_tran rttran(s_value *p) {
  if (p->type == TRAN)
    return p->tran;
  else {
    printf("Not a translation: ");
    print(p);
  }
}

s_value *car(s_value * p) {
  if (p->type == CONS)
    return p->cons.curr;
  else {
    printf("Car: not a cons: ");
    print(p);
  }
}

s_value *cdr(s_value * p) {
  if (p->type == CONS)
    return p->cons.next;
  else {
    printf("Cdr: not a cons: ");
    print(p);
  }
}

void set_car(s_value * p, s_value *s) {
  if (p->type == CONS)
    p->cons.curr = s;
  else {
    printf("Set-car: not a cons: ");
    print(p);
  }
}

void set_cdr(s_value * p, s_value *s) {
  if (p->type == CONS)
    p->cons.next = s;
  else {
    printf("Set-cdr: not a cons: ");
    print(p);
  }
}

void prnt(s_value *node) {
  if (node == 0) {
    printf("()");
    return;
  }
  switch (node->type) {
  case ATOM:
    printf("%s",node->atom.val);
    break;
  case CONS:
    printf("(");
    s_value *cur = node;
    if (car(node))
      prnt(car(node));
    while (cdr(cur) != 0) {
      cur = cdr(cur);
      if (cur->type == CONS) {
	if (car(cur)) {
	  printf(" ");
	  prnt(car(cur));
	}
      } else {
	printf(" . ");
	prnt(cur);
	break;
      }
    }
    printf(")");
    break;
  case BOOL:
    printf("%s", node->sbln.val ? "#t" : "#f");
    break;
  case INTG:
    printf("%d", node->intg.val);
    break;
  case PROC:
    printf("<PROCEDURE ");
    prnt(node->proc.args);
    printf(" ");
    prnt(node->proc.body);
    printf(">");
    break;
  case BLTN:
    printf("<BUILTIN>");
    break;
  case TRAN:
    printf("<TRANSLATION ");
    prnt(node->tran.rules);
    printf(" ");
    prenv(node->tran.lits);
    printf(">");
    break;
  default:
    printf("<?? %d>", node->type);
  }
}

void print(s_value *node) {
  prnt(node);
  printf("\n");
}

void print_pair(char *key, s_value *value, int u) {
  printf(" %s:", key);
  prnt(value);
}

void prenv(s_environ *env) {
  printf("(==");
  g_hash_table_foreach(env->contents, print_pair, 0);
  /*if (env->parent) {
    printf("Parent:\n");
    print_env(env->parent);
    }*/
  printf(" ==)");
}

void print_env(s_environ *env) {
  prenv(env);
  printf("\n");
}

#ifdef NDEBUG
void dprint(s_value *node) { return; }

void dprint_env(s_environ *env) { return; }

#else

void dprint(s_value *node) {
  print(node);
}

void dprint_env(s_environ *env) {
  print_env(env);
}

#endif
