#include "sqlitequery.h"
#include "bridge.h"
#include "mruby.h"
#include "sqlite3.h"
#include <stdint.h>

void sqlite_query_dfree(mrb_state *, void *ptr) { sqlite3_finalize(ptr); }

mrb_data_type SQLiteQuery_DT = {.struct_name = "sqlite_query",
                                .dfree = sqlite_query_dfree};

[[noreturn]] mrb_value SQLiteQuery_NoConsError(mrb_state *mrb, mrb_value) {
  api->mrb_raise(mrb, class_NoConsError,
                 "tried to construct an SQLite::Query object directly, use the "
                 "SQLite::Connection#query method instead");
  __builtin_unreachable();
}

mrb_value SQLiteQuery_HasColumnsP(mrb_state *mrb, mrb_value vself) {
  sqlite3_stmt *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteQuery_DT);

  if (self == nullptr)
    return mrb_false_value();

  return mrb_bool_value(sqlite3_column_count(self) != 0);
}

mrb_value SQLiteQuery_FinishedP(mrb_state *mrb, mrb_value vself) {
  sqlite3_stmt *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteQuery_DT);

  return mrb_bool_value(sqlite3_data_count(self) == 0);
}

mrb_value SQLiteQuery_GetRubyValueOfColumn(mrb_state *mrb, sqlite3_stmt *stmt,
                                           int col) {
  int type = sqlite3_column_type(stmt, col);

  switch (type) {
  case SQLITE_INTEGER:
    return api->mrb_int_value(mrb, sqlite3_column_int64(stmt, col));
  case SQLITE_FLOAT:
    return api->mrb_float_value(mrb, sqlite3_column_double(stmt, col));
  case SQLITE_TEXT:
    return api->mrb_str_new_cstr(mrb,
                                 (const char *)sqlite3_column_text(stmt, col));
  case SQLITE_BLOB:
    return api->mrb_str_new(mrb, (const char *)sqlite3_column_blob(stmt, col),
                            sqlite3_column_bytes(stmt, col));
  case SQLITE_NULL:
    return api->mrb_nil_value();
  default:
    api->mrb_raise(mrb, class_BaseError,
                   "unknown column type returned by sqlite3_column_type");
    __builtin_unreachable();
  }
}

mrb_value SQLiteQuery_Step(mrb_state *mrb, mrb_value vself) {
  sqlite3_stmt *self = api->mrb_data_check_get_ptr(mrb, vself, &SQLiteQuery_DT);

  if (self == nullptr)
    SQLite_MethodCalledOnUninitializedObject(mrb);

  int ec = sqlite3_step(self);

  if (ec != SQLITE_ROW && ec != SQLITE_DONE && ec != SQLITE_BUSY) {
    mrb_raise(mrb, class_BaseError, "error stepping through query results");
  }

  int cols = sqlite3_column_count(self);

  mrb_value row = api->mrb_iv_get(mrb, vself, sym_iv_row);
  for (int32_t i = 0; i < cols; ++i) {
    mrb_value val = SQLiteQuery_GetRubyValueOfColumn(mrb, self, i);
    api->mrb_ary_set(mrb, row, i, val);
  }

  return api->mrb_ary_new_from_values(
      mrb, 2,
      (mrb_value *)&(const mrb_value[2]){mrb_bool_value(ec == SQLITE_ROW),
                                         row});
}

void SQLiteQuery_Init(mrb_state *mrb) {
  class_SQLiteQuery = DefineSQLiteClass(mrb, sym_Query);
  MRB_SET_INSTANCE_TT(class_SQLiteQuery, MRB_TT_DATA);

  api->mrb_define_class_method_id(mrb, class_SQLiteQuery, sym_new,
                                  SQLiteQuery_NoConsError, MRB_ARGS_ANY());
  api->mrb_define_class_method_id(mrb, class_SQLiteQuery, sym_allocate,
                                  SQLiteQuery_NoConsError, MRB_ARGS_ANY());
  api->mrb_define_method_id(mrb, class_SQLiteQuery, sym_initialize,
                            SQLiteQuery_NoConsError, MRB_ARGS_ANY());
}
