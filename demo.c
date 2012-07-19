#include <malloc.h>
#include <string.h>
#include <glib.h>
#include "mscheme.h"

int main() {
  s_value *integer = mkintg(7);
  printf("%d",integer->sint.v);
  s_value *prog = mkcons(integer,0);
  s_value *next = mkcons(prog,0);
  prog->cons.n = next;
}
