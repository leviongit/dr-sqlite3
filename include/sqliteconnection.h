#ifndef SQLITECONTEXT_H
#define SQLITECONTEXT_H

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"

extern struct RClass *class_SQLiteConnection;
extern mrb_data_type SQLiteConnection_DT;

void SQLiteConnection_Init(mrb_state *mrb);

#endif