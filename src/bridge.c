#include "bridge.h"
#include "dragonruby.h"
#include "mruby.h"
#include "sqlite3.h"
#include "sqliteconnection.h"
#include "sqlitequery.h"

#define LIBDRSQLITE3_VERSION "0.0.1"

struct drb_api_t *api;
struct RClass *class_SQLite;
struct RClass *class_BaseError;
struct RClass *class_UninitObject;
struct RClass *class_NoConsError;
struct RClass *class_ReturnCode;
void SQLite_InitSymbols(mrb_state *mrb);

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

  SQLite_InitSymbols(mrb);

  class_SQLite = api->mrb_define_module_id(mrb, sym_SQLite);

  mrb_value vSQLite = api->mrb_obj_value(class_SQLite);

  class_BaseError = DefineSQLiteClassSuper(
      mrb, sym_BaseError, api->mrb_exc_get_id(mrb, sym_StandardError));
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
