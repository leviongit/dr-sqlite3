#include "bridge.h"
#include "mruby.h"

#define SYMX(NAME, ...) mrb_sym sym_##NAME;
#include "syms.inc"
#undef SYMX

void SQLite_InitSymbols(mrb_state *mrb) {
#define SYMX(NAME, VALUE)                                                      \
  sym_##NAME = api->mrb_intern_static(mrb, #VALUE, STRING_LEN(#VALUE));
#include "syms.inc"
#undef SYMX
}
