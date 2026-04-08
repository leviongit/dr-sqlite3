#include "bridge.h"
#include "dragonruby.h"
#include "mruby.h"
#include "sqlite3.h"
#include "sqliteconnection.h"
#include "sqlitequery.h"

#define STRING_LEN(STR) (sizeof(STR) - 1)
#define LIBDRSQLITE3_VERSION "0.0.1"

[[noreturn, gnu::cold, clang::noinline]] void
SQLite_MethodCalledOnUninitializedObject(mrb_state *mrb) {
  api->mrb_raise(mrb, class_UninitObject,
                 "method called on an uninitialized object :<");
  __builtin_unreachable();
}

DRB_FFI_EXPORT
void drb_register_c_extensions_with_api(mrb_state *mrb,
                                        struct drb_api_t *api_) {
  api = api_;
#define SYMX(NAME, VALUE)                                                      \
  sym_##NAME = api->mrb_intern_static(mrb, #VALUE, STRING_LEN(#VALUE));
#include "syms.inc"
#undef SYMX

  mrb_p(mrb, api->mrb_symbol_value(sym_VERSION));

  class_SQLite = api->mrb_define_module_id(mrb, sym_SQLite);

  mrb_value vSQLite = api->mrb_obj_value(class_SQLite);

  class_BaseError =
      DefineSQLiteClassSuper(mrb, sym_BaseError, mrb->eStandardError_class);
  class_UninitObject =
      DefineSQLiteClassSuper(mrb, sym_UninitObject, class_BaseError);

  api->mrb_const_set(
      mrb, vSQLite, sym_VERSION,
      mrb_str_new_static(mrb, SQLITE_VERSION, STRING_LEN(SQLITE_VERSION)));

  api->mrb_const_set(mrb, vSQLite, sym_LIB_VERSION,
                     api->mrb_str_new_static(mrb, LIBDRSQLITE3_VERSION,
                                             STRING_LEN(LIBDRSQLITE3_VERSION)));

  SQLiteConnection_Init(mrb);
  SQLiteQuery_Init(mrb);
}
