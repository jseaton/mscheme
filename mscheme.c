#include <string.h>
#include <glib.h>
#include <stdio.h>
#include "mscheme.h"

s_environ *bind_proc(s_value *proc, s_value *args, s_environ *env) {
  s_environ *new_env = env_new(rtproc(proc).env);	
  s_value *cur_arg  = args;
  s_value *cur_name = rtproc(proc).args;
  if (cur_name && cur_name->type == ATOM) {
    while(cur_arg) {
      set_car(cur_arg,eval(car(cur_arg),env));
      cur_arg = cdr(cur_arg);
    }
    env_add(new_env,rtatom(cur_name).val,args);
  } else {
    while (cur_arg && cur_name && car(cur_arg) && car(cur_name)) {
      print(cur_arg);
      print(cur_name);
      env_add(new_env,rtatom(car(cur_name)).val,eval(car(cur_arg),env));
      cur_arg  = cdr(cur_arg);
      cur_name = cdr(cur_name);
    }
  }
  return new_env;
}

s_value *b_lambda(s_value *args, s_environ *env) {
  return lambda(car(cdr(args)),car(args),env);
}

char bind_pattern(s_value *tran, s_value *node, s_value *args, s_environ *bindings) {
  pmesg("Bind pattern: "); 
  dprint(tran);
  dprint(node);
  dprint(args);
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
  pmesg("Writing pattern: "); dprint(node); dprint_env(env);
  if (node == 0)
    return 0;
  if (node->type == ATOM) {
    s_value *r = env_get(env,node->atom.val);
    if (r)
      return r;
    mkatom(node->atom.val);
  } else if (node->type == CONS) {
    return mkcons(write_pattern(tran, car(node), env), write_pattern(tran, cdr(node), env));
  }
  pmesg("Unknown value in write_pattern\n");
}

s_value *eval_tran(s_value *tran, s_value *args, s_environ *env) {
  s_value *bindings;
  s_value *cur_rule = tran->tran.rules;
  while (1) {
    bindings = env_new(0);
    if (bind_pattern(tran,cdr(car(cur_rule)),args,bindings))
      break;
    if (!cdr(cur_rule))
      pmesg("No matching rules!\n");
    cur_rule = cdr(cur_rule);
  }
  pmesg("Binding made: \n");
  dprint(cur_rule);
  dprint_env(bindings);
  return write_pattern(tran, car(cdr(cur_rule)), bindings);
}

s_value *map_cons(s_value *arg, s_environ *env, s_value *(*fun)(s_value *, s_environ *)) {
  if (arg == 0)
    return 0;
  s_value *cur = arg;
  s_value *r   = mkcons(fun(car(arg),env),0);
  s_value *h   = r;
  while (cur = cdr(cur)) {
    r->cons.next = mkcons(fun(car(cur),env),0);
    r = cdr(r);
  }
  pmesg("Mapped: ");
  dprint(h);
  return h;
}

s_value *eval(s_value *prog, s_environ *env) {
  pmesg("New eval: ");
  dprint(prog);
  //dprint_env(env);
  while (1) {
    pmesg("Eval: ");
    dprint(prog);
    //dprint_env(env);
    if (prog == 0)
      return 0;
    if (prog->type == CONS) {
      s_value *head = eval(car(prog), env);
      if (head == 0)
	pmesg("NULL at head!");
      switch (head->type) {
      case PROC:
	pmesg("Proc: ");
	dprint(head);
	pmesg("Args: ");
	dprint(cdr(prog));
	env  = bind_proc(head, cdr(prog), env);
	prog = head->proc.body;
	break;
      case BLTN:
	pmesg("Bltn: ");
	dprint(cdr(prog));
	return (*head->bltn.fun)(cdr(prog), env);
      case TAIL:
	pmesg("Tail: ");
	dprint(cdr(prog));
	prog = (*head->bltn.fun)(cdr(prog), env);
	break;
      case TRAN:
	pmesg("Tran: ");
	dprint(cdr(prog));
	return eval(eval_tran(head, cdr(prog), env), env);
      default:
	pmesg("Not a proc: ");
	dprint(prog);
	dprint(head);
	return;
      }
    } else if (prog->type == ATOM) {
      return env_get(env,prog->atom.val);
    } else
      return prog;
  }
}

s_value *b_quote(s_value *args, s_environ *env) {
  return args->cons.curr;
}

s_value *t_if(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    pmesg("If: Argument error\n");
  s_value *t = eval(args->cons.curr,env);
  if (t->type != BOOL || t->sbln.val)
    return car(cdr(args));
  else
    return car(cdr(cdr(args)));
}

s_value *b_define(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    pmesg("Define: Argument error\n");
  if (car(args)->type == CONS) {
    env_add(env,car(car(args))->atom.val,lambda(car(cdr(args)),cdr(car(args)),env));
  } else if (car(args)->type == ATOM)
    env_add(env,car(args)->atom.val,eval(car(cdr(args)),env));
  else
    pmesg("Define: Argument error\n");
  return 0;
}

s_value *b_display(s_value *args, s_environ *env) {
  if (args == 0 || args->type != CONS)
    pmesg("Display: Argument error\n");
  print(eval(car(args), env));
  return 0;
}

s_value *b_cons(s_value *args, s_environ *env) {
  return mkcons(eval(car(args),env),eval(car(cdr(args)),env));
}

s_value *b_car(s_value *args, s_environ *env) {
  return car(eval(car(args),env));
}

s_value *b_cdr(s_value *args, s_environ *env) {
  return cdr(eval(car(args),env));
}

s_value *b_set_car(s_value *args, s_environ *env) {
  set_car(eval(car(args),env),eval(car(cdr(args)),env));
  return 0;
}

s_value *b_set_cdr(s_value *args, s_environ *env) {
  set_cdr(eval(car(args),env),eval(car(cdr(args)),env));
  return 0;
}

s_value *t_begin(s_value *args, s_environ *env) {
  s_value *cur = args;
  while(cdr(cur)) {
    eval(car(cur),env);
    cur = cdr(cur);
  }
  return car(cur);
}

s_value *b_set(s_value *args, s_environ *env) {
  env_set(env, car(args)->atom.val, eval(car(cdr(args)),env));
  return 0;
}

void quasiquote(s_value *args, s_environ *env, s_value *parent, s_value *pred) {
  s_value *cur = args;
  if (!(cur && car(cur)))
    return;
  if (car(args)->type == ATOM) {
    if (strcmp(car(args)->atom.val,"unquote") == 0) {
      parent->cons.curr = eval(car(cdr(args)),env);
      return;
    } else if (pred && strcmp(car(args)->atom.val,"unquote-splicing") == 0) {
      s_value *insert = eval(car(cdr(args)),env);
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

s_value *b_define_syntax(s_value *args, s_environ *env) {
  s_value *srules = cdr(car(cdr(args)));
  pmesg("Srules: "); dprint(srules);
  env_add(env,car(args)->atom.val,mktran(car(srules),car(cdr(srules))));
  return 0;
}

s_value *b_eq(s_value *args, s_environ *env) {
  s_value *r = eval(car(args),env);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    if (r != eval(car(cur),env))
      return mkbool(0);
  }
  return mkbool(1);
}

s_value *b_plus(s_value *args, s_environ *env) {
  s_value *r = mkintg(eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    r->intg.val += eval(car(cur),env)->intg.val;
  }
  return r;
}

s_value *b_times(s_value *args, s_environ *env) {
  s_value *r = mkintg(eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    r->intg.val *= eval(car(cur),env)->intg.val;
  }
  return r;
}

s_value *b_equal(s_value *args, s_environ *env) {
  s_value *r = mkintg(eval(car(args),env)->intg.val);
  s_value *cur = args;
  while(cdr(cur)) {
    cur = cdr(cur);
    if (r->intg.val != eval(car(cur),env)->intg.val)
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
  env_add(env,"if",mktail(&t_if));
  env_add(env,"display",mkbltn(&b_display));
  env_add(env,"cons",mkbltn(&b_cons));
  env_add(env,"car",mkbltn(&b_car));
  env_add(env,"cdr",mkbltn(&b_cdr));
  env_add(env,"set-car!",mkbltn(&b_set_car));
  env_add(env,"set-cdr!",mkbltn(&b_set_cdr));
  env_add(env,"quasiquote",mkbltn(&b_quasiquote));
  env_add(env,"define-syntax",mkbltn(&b_define_syntax));
  env_add(env,"set!",mkbltn(&b_set));
  env_add(env,"begin",mktail(&t_begin));
  env_add(env,"eq?",mkbltn(&b_eq));
  env_add(env,"+",mkbltn(&b_plus));
  env_add(env,"*",mkbltn(&b_times));
  env_add(env,"=",mkbltn(&b_equal));

  return env;
}

