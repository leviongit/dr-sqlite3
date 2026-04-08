#include "sqliteconnection.h"
#include "bridge.h"
#include "mruby.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "sqlite3.h"
#include "sqlitequery.h"

void dfree_sqlite(mrb_state *, void *sqlite) { sqlite3_close_v2(sqlite); }

struct RClass *class_SQLiteConnection;
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

  const char *dbname = ":memory:";
  mrb_int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

  api->mrb_get_args(mrb, "|zi", &dbname, &flags);

  sqlite3 *sql;
  int ec = sqlite3_open_v2(dbname, &sql, flags, nullptr);

  if (ec != SQLITE_OK) {
    api->mrb_raise(mrb, class_BaseError,
                   "failed to allocate sqlite connection");
  }

  sql_data->data = sql;

  return self;
}

mrb_value SQLiteConnection_Query(mrb_state *mrb, mrb_value vself) {
  sqlite3 *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteConnection_DT);

  if (self == nullptr)
    SQLite_MethodCalledOnUninitializedObject(mrb);

  mrb_value arg = api->mrb_get_arg1(mrb);

  if (!mrb_string_p(arg)) {
    api->mrb_raise(mrb, api->mrb_exc_get_id(mrb, sym_ArgumentError),
                   "expected String for argument of SQLite::Connection#query");
  }

  struct RString *sql = RSTRING(arg);
  sqlite3_stmt *stmt;

  const char *tail = nullptr;

  int ec = sqlite3_prepare_v2(self, RSTR_PTR(sql), RSTR_LEN(sql), &stmt, &tail);

  if (ec != SQLITE_OK) {
    api->mrb_raise(mrb, class_BaseError, "failed to compile query");
  }
  if (tail != nullptr && *tail != '\0') {
    sqlite3_finalize(stmt);
    api->mrb_raise(
        mrb, class_BaseError,
        "multiple statements are not supported by SQLite::Connection#query, "
        "but the provided SQL string contains multiple statements");
  }

  struct RData *query =
      api->mrb_data_object_alloc(mrb, class_SQLiteQuery, stmt, &SQLiteQuery_DT);

  int cols = sqlite3_column_count(stmt);

  mrb_value row = api->mrb_ary_new_capa(mrb, cols);

  api->mrb_obj_iv_set(mrb, (struct RObject *)query, sym_iv_row, row);

  return api->mrb_obj_value(query);
}

mrb_value SQLiteConnection_Exec(mrb_state *mrb, mrb_value vself) {
  sqlite3 *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteConnection_DT);

  if (self == nullptr)
    SQLite_MethodCalledOnUninitializedObject(mrb);

  mrb_value arg = api->mrb_get_arg1(mrb);

  if (!mrb_string_p(arg)) {
    api->mrb_raise(mrb, api->mrb_exc_get_id(mrb, sym_BaseError),
                   "expected String for argument of SQLite::Connection#exec");
  }

  struct RString *sql = RSTRING(arg);
  char *err = nullptr;

  int ec = sqlite3_exec(self, RSTR_PTR(sql), nullptr, nullptr, &err);

  if (ec != SQLITE_OK) {
    mrb_value err_str = api->mrb_str_new_cstr(mrb, err);
    sqlite3_free(err);
    api->mrb_raise(mrb, class_BaseError, RSTRING_PTR(err_str));
  }

  return api->mrb_nil_value();
}

void SQLiteConnection_Init(mrb_state *mrb) {
  class_SQLiteConnection = DefineSQLiteClass(mrb, sym_Connection);
  MRB_SET_INSTANCE_TT(class_SQLiteConnection, MRB_TT_DATA);

  api->mrb_define_class_method_id(mrb, class_SQLiteConnection, sym_allocate,
                                  SQLiteConnection_Allocate, MRB_ARGS_NONE());
  api->mrb_define_method_id(mrb, class_SQLiteConnection, sym_initialize,
                            SQLiteConnection_Initialize,
                            MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));
  api->mrb_define_method_id(mrb, class_SQLiteConnection, sym_query,
                            SQLiteConnection_Query, MRB_ARGS_REQ(1));
  api->mrb_define_method_id(mrb, class_SQLiteConnection, sym_exec,
                            SQLiteConnection_Exec, MRB_ARGS_REQ(1));
}
