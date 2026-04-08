#include "sqlitequery.h"
#include "bridge.h"
#include "mruby.h"
#include "sqlite3.h"

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

void SQLiteQuerty_Init(mrb_state *mrb) {
  class_SQLiteQuery = DefineSQLiteClass(mrb, sym_Query);
  MRB_SET_INSTANCE_TT(class_SQLiteQuery, MRB_TT_DATA);

  api->mrb_define_class_method_id(mrb, class_SQLiteQuery, sym_new,
                                  SQLiteQuery_NoConsError, MRB_ARGS_ANY());
  api->mrb_define_class_method_id(mrb, class_SQLiteQuery, sym_allocate,
                                  SQLiteQuery_NoConsError, MRB_ARGS_ANY());
  api->mrb_define_method_id(mrb, class_SQLiteQuery, sym_initialize,
                            SQLiteQuery_NoConsError, MRB_ARGS_ANY());
}
