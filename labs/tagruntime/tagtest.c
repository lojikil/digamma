/* Simple tagged runtime test
 * Copyright 2011, Stefan Edwards, released under
 * the zlib/png license. See the LICENSE file
 * for details
 */

#include <stdio.h>
#include <gc.h>

#define nil NULL

#define TYPE(x) ((x) & 0x20)
#define SET_TYPE(x) 0
#define NUMBERP(x) (TYPE(x) == T_INTEGER || TYPE(x) == T_RATIONAL\
                    TYPE(X) == T_REAL || TYPE(x) == T_DREAL \
                    TYPE(X) == T_COMPLEX)

#define SNIL 0l
#define SVOID T_VOID

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
SExp fprimadd(SExp,SExp);
SExp fprimsub(SExp,SExp);
SExp fprimmul(SExp,SExp);
SExp fprimdiv(SExp,SExp);
SExp fprimgt(SExp,SExp);
SExp fprimgte(SExp,SExp);
SExp fprimlt(SExp,SExp);
SExp fprimlte(SExp, SExp);

int
main()
{
    GC_INIT();
    SExp l = cons(makeint(10),cons(makeint(11),cons(makeint(12),SNIL)));
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

SExp 
princ(SExp o, int mode)
{
    switch(TYPE(o))
    {
        case T_INTEGER:
            printf("%d",AINT(o));
            break;
        case T_REAL:
            Number *d = ANUMBER(o);
            printf("%f",AREAL(o));
            break;
        case T_RATIONAL:
            Number *q = ANUMBER(o);
            printf("%d/%d",ANUM(q),ADEN(q));
            break;
        case T_COMPLEX:
            Number *c = ANUMBER(o);
            printf("%f+%fi",AREAL(c),AIMAG(c));
            break;
        case T_PAIR:
            Pair *p = APAIR(o);
            printf("(")
            princ(car(o));
            if(cdr(o) == SNIL)
                printf(")");
            else
                princ(cdr(o));
            break;
        case T_NULL:
            printf("()");
            break;
        case T_KEY: /* in write, T_KEY should prefix #\: */
            if(mode)
                printf(":");
        case T_STRING:
        case T_ATOM:
            String *s = ATRING(o);
            printf("%s",s->str);
            break;
        case T_VECTOR:
            Vector *v = AVECTOR(o);
            printf("[");
            for(int i = 0; i < v->length; i++)
            {
                princ(v->data[i]);
                if(i < (v->length - 1))
                    printf(" ");
            }
            printf("]");
            break;
        case T_ERROR:
        case T_VOID:
            break;
    }
    return SVOID;
}

SExp
fprimadd(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimsub(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimmul(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimdiv(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimgt(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimgte(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimlt(SExp a,SExp b)
{
    return SVOID;
}
SExp
fprimlte(SExp a, SExp b)
{
    return SVOID;
}
