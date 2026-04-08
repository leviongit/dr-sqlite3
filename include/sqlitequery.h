#ifndef SQLITE_STMT_H
#define SQLITE_STMT_H

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"

extern struct RClass *class_SQLiteQuery;
extern mrb_data_type SQLiteQuery_DT;

void SQLiteQuery_Init(mrb_state *mrb);

#endif
