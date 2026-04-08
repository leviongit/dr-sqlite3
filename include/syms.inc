#ifndef SYMX
#error "define SYMX"
#endif

#ifndef SYM
#define SYM_DEFAULT
#define SYM(_S) SYMX(_S, _S)
#endif

SYM(allocate)
SYM(initialize)
SYM(SQLite)
SYM(VERSION)
SYM(LIB_VERSION)
SYM(UninitObject)
SYM(BaseError)
SYM(Connection)
SYM(Query)
SYM(query)
SYM(new)
SYM(NoConsError)
SYM(ArgumentError)
SYM(ReturnCode)
SYMX(iv_row, @row)
SYM(exec)
SYM(StandardError)
SYMX(has_columns_p, has_columns?)
SYMX(finished_p, finished?)
SYM(step)

#ifdef SYM_DEFAULT
#undef SYM_DEFAULT
#undef SYM
#endif
