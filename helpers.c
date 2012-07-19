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
  printf("Variable %s not found\n", key);
  return 0;
}

void env_set(s_environ *env, char *key, s_value *value) {
  s_value *r = g_hash_table_lookup(env->contents, key);
  if (r)
    return env_add(env, key, value);
  if (env->parent)
    return env_set(env->parent, key, value);
  printf("Variable %s not found\n", key);
}	

s_value *mkatom(char *s) {
  s_value *r   = (s_value *)malloc(sizeof(s_atom));
  r->atom.type = ATOM;
  r->atom.val  = s;
  return r;
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

s_value *mkbool(char b) {
  s_value *r   = (s_value *)malloc(sizeof(s_bool));
  r->sbln.type = BOOL;
  r->sbln.val  = b;
  return r;
}

s_value *mkintg(int i) {
  s_value *r   = (s_value *)malloc(sizeof(s_intg));
  r->intg.type = INTG;
  r->intg.val  = i;
  return r;
}

s_value *lambda(s_value *body, s_value *args, s_environ *env) {
  s_value *r   = (s_value *)malloc(sizeof(s_proc));
  r->proc.type = PROC;
  r->proc.env  = env;
  r->proc.args = args;
  r->proc.body = body;
  return r;
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
      printf(" ");
      cur = cdr(cur);
      if (car(cur))
	prnt(car(cur));
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
