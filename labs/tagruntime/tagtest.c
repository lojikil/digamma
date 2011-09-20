/* Simple tagged runtime test
 * Copyright 2011, Stefan Edwards, released under
 * the zlib/png license. See the LICENSE file
 * for details
 */

#include <stdio.h>
#include <gc.h>

#define nil NULL

#define TYPE(x) ((long)(x) & 0x20)
#define SET_TYPE(x) 0
#define NUMBERP(x) (TYPE(x) == T_INTEGER || TYPE(x) == T_RATIONAL\
                    TYPE(X) == T_REAL || TYPE(x) == T_DREAL \
                    TYPE(X) == T_COMPLEX)

typedef enum {
    T_NULL,T_INTEGER, T_RATIONAL, T_REAL, T_COMPLEX,
    T_VOID, T_STRING, T_PAIR, T_ATOM, T_KEY, T_VECTOR,
    T_DICT, T_CLOSURE, T_PROCEDURE, T_FOREIGN, T_ERROR,
    T_SYNTAX, T_MACRO
} SExpType;

typedef struct 
{
    char *data;
    int len;
} String;

typedef struct 
{
    SExp head;
    SExp rest;
} Pair;

typedef struct
{
    int len;
    SExp *data; /* hmalloc'd list of SExps of length len */
} Vector;

typedef union
{
    double r;
    struct
    {
        int num;
        int den;
    } rat;
    struct
    {
        double real;
        double imag;
    } comp;
} Number;

typedef long long SExp;

SExp makeint(int);
SExp makebool(char c);
SExp makegoal(char c);
SExp makestring(char *);
SExp cons(SExp, SExp);
SExp car(SExp);
SExp cdr(SExp);
SExp princ(SExp);

int
main()
{
    SExp *l;
    GC_INIT();
    return 0;
}

SExp
makeinteger(int i)
{
    SExp ret;
    SET_INT(ret,i);
    SET_TYPE(ret,T_INTEGER);
    return ret;
}

SExp
makestring(char *s)
{
    SExp ret;
    String *r = hmalloc(sizeof(String));
    r->data = s;
    r->len = strlen(s);
    SET_PTR(ret,r);
    SET_TYPE(ret,T_STRING);
    return ret;
}
SExp
cons(SExp a, SExp b)
{
    SExp ret;
    SET_TYPE(ret,T_PAIR);
    return ret;
}

SExp
car(SExp o)
{
    if(TYPEP(o,T_PAIR))
    {
        return nil;
    }
    return nil;
}

SExp
cdr(SExp o)
{
    if(TYPEP(o,T_PAIR))
    {
        return SNULL;
    }
    return SNULL;
}
