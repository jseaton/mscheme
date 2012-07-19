#include <string.h>
#include <glib.h>
#include <stdio.h>
#include "mscheme.h"
#include "parser.tab.h"



s_value *eval_proc(s_value *proc, s_value *args, s_environ *env) {
  s_environ *new_env = env_new(proc->proc.env);	
  s_value *cur_arg  = args;
  s_value *cur_name = proc->proc.args;
  if (cur_name->type == ATOM) {
    while(cur_arg) {
      cur_arg->cons.curr = b_eval(car(cur_arg),env);
      cur_arg = cdr(cur_arg);
    }
    env_add(new_env,cur_name->atom.val,args);
  } else {
    while (cur_arg && cur_name) {
      env_add(new_env,car(cur_name)->atom.val,b_eval(car(cur_arg),env));
      cur_arg  = cdr(cur_arg);
      cur_name = cdr(cur_name);
    }
  }
  return b_eval(proc->proc.body, new_env);
}

s_value *b_lambda(s_value *args, s_environ *env) {
  return lambda(car(cdr(args)),car(args),env);
}

char bind_pattern(s_value *tran, s_value *node, s_value *args, s_environ *bindings) {
  printf("Bind pattern: "); print(tran); print(node); print(args);
  if (node == 0)
    return 0;
  if (node->type == ATOM) {
    if (env_get(tran->tran.lits,node->atom.val)) {
      if (args->type != ATOM || strcmp(node->atom.val,args->atom.val))
	return 0;
    } else {
      env_add(bindings,node->atom.val,args);
    }
  } else if (node->type == CONS) {
    if (args->type != CONS)
      return 0;
    bind_pattern(tran, car(node), car(args), bindings);
    bind_pattern(tran, cdr(node), cdr(args), bindings);
  } else {
    return 0; //?
  }
  return 1;
}

s_value *write_pattern(s_value *tran, s_value *node, s_environ *env) {
  printf("Writing pattern: "); print(node); print_env(env);
  if (node == 0)
    return 0;
  if (node->type == ATOM) {
    s_value *r = env_get(env,node->atom.val);
    if (r)
      return r;
    if (env_get(tran->tran.lits,node->atom.val))
      return mkatom(node->atom.val);
  } else if (node->type == CONS) {
    return mkcons(write_pattern(tran, car(node), env), write_pattern(tran, cdr(node), env));
  }
  printf("Unknown value in write_pattern\n");
}

s_value *eval_tran(s_value *tran, s_value *args, s_environ *env) {
  s_value *bindings;
  s_value *cur_rule = tran->tran.rules;
  while (1) {
    bindings = env_new(0);
    if (bind_pattern(tran,cdr(car(cur_rule)),args,bindings))
      break;
    if (!cdr(cur_rule))
      printf("No matching rules!\n");
    cur_rule = cdr(cur_rule);
  }
  printf("Binding made: \n");
  print(cur_rule);
  print_env(bindings);
  return write_pattern(tran, car(cdr(cur_rule)), bindings);
}

s_value *b_eval(s_value *prog, s_environ *env) {
  print_env(env);
  if (prog == 0)
    return 0;
  if (prog->type == CONS) {
    s_value *head = b_eval(car(prog), env);
    if (head == 0)
      printf("NULL at head!");
    switch (head->type) {
    case PROC:
      printf("Proc: ");
      print(head);
      printf("Args: ");
      print(cdr(prog));
      return eval_proc(head, cdr(prog), env);
    case BLTN:
      printf("Bltn: ");
      print(cdr(prog));
      return (*head->bltn.fun)(cdr(prog), env);
    case TRAN:
      printf("Tran: ");
      print(cdr(prog));
      return b_eval(eval_tran(head, cdr(prog), env), env);
    default:
      printf("Not a proc!");
    }
  } else if (prog->type == ATOM) {
    printf("Retrieving %s: ",prog->atom.val);
    s_value *p = env_get(env,prog->atom.val);
    print(p);
    return p;
  } else
    return prog;
}

s_value *b_quote(s_value *args, s_environ *env) {
  return args->cons.curr;
}

s_value *b_if(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    printf("If: Argument error\n");
  s_value *t = b_eval(args->cons.curr,env);
  if (t->type != BOOL || t->sbln.val)
    return b_eval(car(cdr(args)),env);
  else
    return b_eval(car(cdr(cdr(args))),env);
}

s_value *b_define(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    printf("Define: Argument error\n");
  if (car(args)->type == CONS) {
    env_add(env,car(car(args))->atom.val,lambda(car(cdr(args)),cdr(car(args)),env));
  } else if (car(args)->type == ATOM)
    env_add(env,car(args)->atom.val,b_eval(car(cdr(args)),env));
  else
    printf("Define: Argument error\n");
  return 0;
}

s_value *b_display(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    printf("Display: Argument error\n");
  print(b_eval(car(args), env));
  return 0;
}

s_value *b_cons(s_value *args, s_environ *env) {
  return mkcons(b_eval(car(args),env),b_eval(car(cdr(args)),env));
}

s_value *b_car(s_value *args, s_environ *env) {
  return car(b_eval(car(args),env));
}

s_value *b_cdr(s_value *args, s_environ *env) {
  return cdr(b_eval(car(args),env));
}

s_value *b_begin(s_value *args, s_environ *env) {
  s_value *r = b_eval(car(args),env);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    r = b_eval(car(cur),env);
  }
  return r;
}

s_value *b_set(s_value *args, s_environ *env) {
  env_set(env, car(args)->atom.val, b_eval(car(cdr(args)),env));
  return 0;
}

void quasiquote(s_value *args, s_environ *env, s_value *parent, s_value *pred) {
  s_value *cur = args;
  if (car(args)->type == ATOM) {
    if (strcmp(car(args)->atom.val,"unquote") == 0) {
      parent->cons.curr = b_eval(car(cdr(args)),env);
      return;
    } else if (pred && strcmp(car(args)->atom.val,"unquote-splicing") == 0) {
      s_value *insert = b_eval(car(cdr(args)),env);
      pred->cons.next = insert;
      while (cdr(insert))
	insert = cdr(insert);
      insert->cons.next = cdr(parent);
      return;
    }
  }
  while(1) {
    if (!(cur && cdr(cur)))
      break;
    if (car(cdr(cur))->type == CONS)
      quasiquote(car(cdr(cur)),env,cdr(cur), cur);
    cur = cdr(cur);
  }
  return;
}

s_value *b_quasiquote(s_value *args, s_environ *env) {
  if (car(args)->type == CONS)
    quasiquote(car(args),env,args,0);
  return car(args);
}

s_value *mktran(s_value *lits, s_value *rules) {
  printf("Mktran: ");
  print(lits);
  print(rules);
  s_value *r   = (s_value *)malloc(sizeof(s_tran));
  r->tran.type = TRAN;
  r->tran.lits  = env_new(0);
  r->tran.rules = rules;
  while (lits && car(lits)) {
    env_add(r->tran.lits,car(lits)->atom.val,mkbool(1));
    lits = cdr(lits);
  }
  printf("----\n");
  print_env(r->tran.lits);
  return r;
}

s_value *b_define_syntax(s_value *args, s_environ *env) {
  s_value *srules = cdr(car(cdr(args)));
  printf("Srules: "); print(srules);
  env_add(env,car(args)->atom.val,mktran(car(srules),car(cdr(srules))));
  return 0;
}

s_value *b_eq(s_value *args, s_environ *env) {
  s_value *r = b_eval(car(args),env);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    if (r != b_eval(car(cur),env))
      return mkbool(0);
  }
  return mkbool(1);
}

s_value *b_plus(s_value *args, s_environ *env) {
  s_value *r = mkintg(b_eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    r->intg.val += b_eval(car(cur),env)->intg.val;
  }
  return r;
}

s_value *b_times(s_value *args, s_environ *env) {
  s_value *r = mkintg(b_eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    r->intg.val *= b_eval(car(cur),env)->intg.val;
  }
  return r;
}

s_value *b_equal(s_value *args, s_environ *env) {
  s_value *r = mkintg(b_eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    if (r->intg.val != b_eval(car(cur),env)->intg.val)
      return mkbool(0);
  }
  return mkbool(1);
}

void string_eval(char *raw_input, s_environ *env) {
  char *input = yy_scan_string(raw_input);
  yyparse(env);
  yy_delete_buffer(input);
}

s_environ *global_env() {
  s_environ *env = env_new(0);
  env_add(env,"lambda",mkbltn(&b_lambda));
  env_add(env,"define",mkbltn(&b_define));
  env_add(env,"quote",mkbltn(&b_quote));
  env_add(env,"if",mkbltn(&b_if));
  env_add(env,"display",mkbltn(&b_display));
  env_add(env,"cons",mkbltn(&b_cons));
  env_add(env,"car",mkbltn(&b_car));
  env_add(env,"cdr",mkbltn(&b_cdr));
  env_add(env,"quasiquote",mkbltn(&b_quasiquote));
  env_add(env,"define-syntax",mkbltn(&b_define_syntax));
  env_add(env,"set!",mkbltn(&b_set));
  env_add(env,"begin",mkbltn(&b_begin));
  env_add(env,"eq?",mkbltn(&b_eq));
  env_add(env,"+",mkbltn(&b_plus));
  env_add(env,"*",mkbltn(&b_times));
  env_add(env,"=",mkbltn(&b_equal));

  return env;
}

int main() {
  char raw_input[200];
  s_environ *env = global_env();

  while (1) {
    printf(">>>> ");
    fgets(raw_input, sizeof raw_input, stdin);
    string_eval(raw_input, env);
    }

  return 0;
}
