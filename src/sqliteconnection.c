#include "sqliteconnection.h"
#include "bridge.h"
#include "mruby.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "sqlite3.h"
#include "sqlitequery.h"

void dfree_sqlite(mrb_state *, void *sqlite) { sqlite3_close_v2(sqlite); }

mrb_data_type SQLiteConnection_DT = {.struct_name = "sqlite",
                                     .dfree = dfree_sqlite};

mrb_value SQLiteConnection_Allocate(mrb_state *mrb, mrb_value klass) {
  api->mrb_check_type(mrb, klass, MRB_TT_CLASS);

  struct RData *data = api->mrb_data_object_alloc(
      mrb, mrb_class_ptr(klass), nullptr, &SQLiteConnection_DT);

  return api->mrb_obj_value(data);
}

mrb_value SQLiteConnection_Initialize(mrb_state *mrb, mrb_value self) {
  struct RData *sql_data = (struct RData *)mrb_obj_ptr(self);
  if (sql_data->data) {
    return self;
  }

  mrb_value vdbname = api->mrb_nil_value();
  mrb_int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

  api->mrb_get_args(mrb, "|si", &vdbname, &flags);

  const char *dbname = (mrb_nil_p(vdbname)) ? ":memory:" : RSTRING_PTR(vdbname);

  sqlite3 *sql;
  int ec = sqlite3_open_v2(dbname, &sql, flags, nullptr);

  if (ec != SQLITE_OK) {
    mrb_raise(mrb, class_BaseError, "failed to allocate sqlite connection");
  }

  sql_data->data = sql;

  return self;
}

mrb_value SQLiteConnection_Query(mrb_state *mrb, mrb_value vself) {
  sqlite3 *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteConnection_DT);

  if (self == nullptr)
    SQLite_MethodCalledOnUninitializedObject(mrb);

  mrb_value arg = mrb_get_arg1(mrb);

  if (!mrb_string_p(arg)) {
    mrb_raise(mrb, api->mrb_exc_get_id(mrb, sym_ArgumentError),
              "expected String for argument of SQLite::Connection#query");
  }

  struct RString *sql = RSTRING(arg);
  sqlite3_stmt *stmt;

  int ec =
      sqlite3_prepare_v2(self, RSTR_PTR(sql), RSTR_LEN(sql), &stmt, nullptr);

  if (ec != SQLITE_OK) {
    mrb_raise(mrb, class_BaseError, "failed to compile query");
  }

  return api->mrb_obj_value(api->mrb_data_object_alloc(mrb, class_SQLiteQuery,
                                                       stmt, &SQLiteQuery_DT));
}

void SQLiteConnection_Init(mrb_state *mrb) {
  class_SQLiteConnection = DefineSQLiteClass(mrb, sym_Connection);
  MRB_SET_INSTANCE_TT(class_SQLiteConnection, MRB_TT_DATA);

  api->mrb_define_class_method_id(mrb, class_SQLiteConnection, sym_allocate,
                                  SQLiteConnection_Allocate, MRB_ARGS_NONE());
  api->mrb_define_method_id(mrb, class_SQLiteConnection, sym_initialize,
                            SQLiteConnection_Initialize,
                            MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));
}