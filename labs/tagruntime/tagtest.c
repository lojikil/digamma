/* Simple tagged runtime test
 * Copyright 2011, Stefan Edwards, released under
 * the zlib/png license. See the LICENSE file
 * for details
 */

#include <stdio.h>
#include <gc.h>

typedef struct 
{
    char *data;
    int len;
} String;

typedef struct 
{
    void *head;
    void *rest;
} Pair;

typedef long long SExp;

SExp *makeint(int);
SExp *makebool(char c);
SExp *makegoal(char c);
SExp *makestring(char *,int);
SExp *cons(SExp *, SExp *);
SExp *car(SExp *);
SExp *cdr(SExp *);
SExp *princ(SExp *);

int
main()
{
    SExp *l;
    GC_INIT();
    return 0;
}

SExp *
makeinteger(int i)
{
    SExp *ret = nil;
    ret = (SExp *)hmalloc(sizeof(SExp));
    AINT(ret) = i;
    TYPE(ret) = TINTEGER;
    return ret;
}
