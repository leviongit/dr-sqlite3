#ifndef BRIDGE_H
#define BRIDGE_H

#include "dragonruby.h"
#include "mruby.h"
#include "mruby/class.h"

extern struct drb_api_t *api;
extern struct RClass *class_SQLite;
extern struct RClass *class_BaseError;
extern struct RClass *class_UninitObject;
extern struct RClass *class_NoConsError;
extern struct RClass *class_ReturnCode;
extern struct RClass *class_ReturnCode_Busy;
extern struct RClass *class_ReturnCode_Done;
extern struct RClass *class_ReturnCode_Error;
extern struct RClass *class_ReturnCode_Misuse;

#define SYM(NAME) extern mrb_sym sym_##NAME;
#include "syms.inc"
#undef SYM

inline static struct RClass *
DefineSQLiteClassSuper(mrb_state *mrb, mrb_sym name, struct RClass *super) {
  return api->mrb_define_class_under_id(mrb, class_SQLite, name, super);
}

inline static struct RClass *DefineSQLiteClass(mrb_state *mrb, mrb_sym name) {
  return DefineSQLiteClassSuper(mrb, name, mrb->object_class);
}

[[noreturn, gnu::cold, clang::noinline]] void
SQLite_MethodCalledOnUninitializedObject(mrb_state *mrb);

#endif