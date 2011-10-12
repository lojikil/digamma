/* Simple tagged runtime test
 * Copyright 2011, Stefan Edwards, released under
 * the zlib/png license. See the LICENSE file
 * for details
 */

#include <stdio.h>
#include <gc.h>
#include <string.h>

#define nil NULL

#define TYPE(x) ((x) & 0xFF)
#define SET_TYPE(o,x) ((o) += (0xC00 + x)) 
#define NUMBERP(x) (TYPE(x) == T_INTEGER || TYPE(x) == T_RATIONAL ||\
                    TYPE(x) == T_REAL || TYPE(x) == T_COMPLEX)

#define AINT(x) (x) >> 32
#define AREAL(x) (x)->r
#define ANUM(x) (x)->rat.num
#define ADEN(x) (x)->rat.den
#define AIMAG(x) (x)->comp.imag
#define ACEREAL(x) (x)->comp.real
#define ANUMBER(x) (Number *)(x >> 32)
#define APAIR(x) (Pair *)(x >> 32)
#define ASTRING(x) (String *)(x >> 32)
#define AVECTOR(x) (Vector *)(x >> 32)
#define SET_NUM(x) (SExp)(x) << 32
#define SET_PTR(x) (SExp)(x) << 32
#define SET_INT(x) (SExp)(x) << 32

#define SNIL 0xc0
#define SVOID (0xc0 + T_VOID)
#define STRUE (0xc0 + T_BOOL) + ((SExp)1l << 32)
#define SFALSE (0xc0 + T_BOOL)
#define SSUCC (0xc0 + T_GOAL) + ((SExp)1l << 32)
#define SUNSUCC (0xc0 + T_GOAL)

#define hmalloc GC_MALLOC

typedef enum {
    T_NULL,T_INTEGER, T_RATIONAL, T_REAL, T_COMPLEX,
    T_VOID, T_STRING, T_PAIR, T_ATOM, T_KEY, T_VECTOR,
    T_DICT, T_CLOSURE, T_PROCEDURE, T_FOREIGN, T_ERROR,
    T_SYNTAX, T_MACRO, T_BOOL, T_GOAL, T_EOF
} SExpType;

typedef struct 
{
    int len;
    char *str;
} String;

typedef long long SExp;

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

SExp makeinteger(int);
SExp makereal(double);
SExp makecomplex(double, double);
SExp makerational(int,int);
SExp makebool(char c);
SExp makegoal(char c);
SExp makestring(char *);
SExp makeerror(int,int,char *);
SExp cons(SExp, SExp);
SExp car(SExp);
SExp cdr(SExp);
SExp princ(SExp,int);
SExp fprimadd(SExp,SExp);
SExp fprimsub(SExp,SExp);
SExp fprimmul(SExp,SExp);
SExp fprimdiv(SExp,SExp);
SExp fprimgt(SExp,SExp);
SExp fprimgte(SExp,SExp);
SExp fprimlt(SExp,SExp);
SExp fprimlte(SExp, SExp);

#ifndef RTONLY
/* if we want a run time, don't include a main */
int
main()
{
    /* need to test:
     * == equality of atomic literals
     * basic math operations on all tagged numerical types
     * basic boolean operations on all tagged numerical types
     * string access
     * list a access
     * printing
     */
    GC_INIT();
    SExp f = makeinteger(10);
    SExp g = makeinteger(10);
    Number *n0 = nil, *n1 = nil;
    
    printf("BASIC Numeric creation checks\n");
    if(f != g)
        printf("integers are not equal!\n");
    else
        printf("PASS integer equality test\n"); 
    f = makereal(1.0);

    if(TYPE(f) != T_REAL)
        printf("incorrect type set for reals\n");
    else
        printf("pass real type check\n");
    n0 = ANUMBER(f);
    if(n0->r != 1.0)
        printf("real-value is not == 1.0\n");
    else
        printf("pass real value check\n");

    g = makerational(3,4);
    if(TYPE(g) != T_RATIONAL)
        printf("incorrect type set for rationals\n");
    else
        printf("pass rational type check\n");
    n0 = ANUMBER(g);
    if(n0->rat.num != 3 || n0->rat.den != 4)
        printf("rational-value is not == 3/4\n");
    else
        printf("pass rational value check\n");
     
    f = makecomplex(1.0,0.75);
    if(TYPE(f) != T_COMPLEX)
        printf("incorrect type set for complex numbers\n");
    else
        printf("pass complex type check\n");
    n0 = ANUMBER(f);
    if(AREAL(n0) != 1.0 || AIMAG(n0) != 0.75)
        printf("incorrect value for complex check\n");
    else
        printf("pass complex value check\n");
    
    printf("\nBASIC Numerical operations checks\n");

    f = fprimadd(makeinteger(1),makeinteger(1));
    if(AINT(f) != 2)
        printf("primitive addition fails for integer\n");
    else
        printf("pass primitive integer addition\n");

    f = fprimadd(makeinteger(1),g);
    n0 = ANUMBER(f);
    if(TYPE(f) != T_RATIONAL)
        printf("primitive addition fails integer + rational type check\n");
    else if(n0->rat.num != 7 || n0->rat.den != 4)
        printf("primitive addtion fails integer + rational value check\n");
    else
        printf("pass primitive addition integer + rational\n");

    /* these two should be the same, but check that primitive addition
     * handles these cases correctly
     */
    f = fprimadd(g,makeinteger(1));
    n0 = ANUMBER(f);
    if(TYPE(f) != T_RATIONAL)
        printf("primitive addition fails rational + integer type check\n");
    else if(n0->rat.num != 7 || n0->rat.den != 4)
        printf("primitive addtion fails rational + integer value check\n");
    else
        printf("pass primitive addition rational + integer\n");

    f = fprimadd(g,makereal(1.0));
    n0 = ANUMBER(f);
    if(TYPE(f) != T_REAL)
        printf("primitive addition fails for rational + real type check\n");
    else if(n0->r != 1.75)
        printf("primitive addition fails for rational + real value check\n");
    else
        printf("pass primitive addition rational + real\n");

    f = fprimadd(g,makecomplex(1.0,0.75));
    n0 = ANUMBER(f);
    if(TYPE(f) != T_COMPLEX)
        printf("primitive addition fails for rational + complex\n");
    else if(n0->comp.real != 1.75 || n0->comp.imag != 0.75)
        printf("primitive addition fails for rational + complex check\n");
    else
        printf("pass primitive addition rational + complex\n");

    f = makestring("this is a test");

    f = cons(f,cons(g,SNIL));
    return 0;
}
#endif 

SExp
makeinteger(int i)
{
    SExp ret;
    ret = SET_INT(i);
    SET_TYPE(ret,T_INTEGER);
    return ret;
}

SExp
makereal(double d)
{
    SExp ret;
    Number *r = hmalloc(sizeof(Number));
    r->r = d;
    ret = SET_PTR(r);
    SET_TYPE(ret,T_REAL);
    return ret;
}

SExp
makerational(int n, int d)
{
    SExp ret;
    Number *q = hmalloc(sizeof(Number));
    q->rat.num = n;
    q->rat.den = d;
    ret = SET_PTR(q);
    SET_TYPE(ret,T_RATIONAL);
    return ret;
}

SExp 
makecomplex(double real, double imag)
{
    SExp ret;
    Number *c = hmalloc(sizeof(Number));
    c->comp.real = real;
    c->comp.imag = imag;
    ret = SET_NUM(c);
    SET_TYPE(ret,T_COMPLEX);
    return ret;
}

SExp
makestring(char *s)
{
    SExp ret;
    String *r = hmalloc(sizeof(String));
    r->str = s;
    r->len = strlen(s);
    ret = SET_PTR(r);
    SET_TYPE(ret,T_STRING);
    return ret;
}
SExp 
makeerror(int syslevel, int layer, char *msg)
{
    return 0l;
}
SExp
cons(SExp a, SExp b)
{
    SExp ret;
    Pair *p = hmalloc(sizeof(Pair));
    p->head = a;
    p->rest = b;
    ret = SET_PTR(p);
    SET_TYPE(ret,T_PAIR);
    return ret;
}

SExp
car(SExp o)
{
    Pair *p = nil;
    if(TYPE(o) == T_PAIR)
    {
        p = APAIR(o);
        return p->head;
    }
    return SNIL;
}

SExp
cdr(SExp o)
{
    Pair *p = nil;
    if(TYPE(o) == T_PAIR)
    {
        p = APAIR(o);
        return p->rest;
    }
    return SNIL;
}

SExp 
princ(SExp o, int mode)
{
    String *s = nil;
    Vector *v = nil;
    Pair *p = nil;
    Number *n = nil;
    int i = 0;
    switch(TYPE(o))
    {
        case T_INTEGER:
            printf("%d",AINT(o));
            break;
        case T_REAL:
            n = ANUMBER(o);
            printf("%f",AREAL(n));
            break;
        case T_RATIONAL:
            n = ANUMBER(o);
            printf("%d/%d",ANUM(n),ADEN(n));
            break;
        case T_COMPLEX:
            n = ANUMBER(o);
            printf("%f+%fi",ACEREAL(n),AIMAG(n));
            break;
        case T_PAIR:
            p = APAIR(o);
            printf("(");
            princ(car(o),0);
            if(cdr(o) == SNIL)
                printf(")");
            else
                princ(cdr(o),0);
            break;
        case T_BOOL:
            if(o == STRUE)
                printf("#t");
            else
                printf("#f");
            break;
        case T_GOAL:
            if(o == SSUCC)
                printf("#s");
            else
                printf("#u");
            break;
        case T_EOF:
            printf("#e");
            break;
        case T_NULL:
            if(mode)
                printf("'");
            printf("()");
            break;
        case T_KEY: /* in write, T_KEY should prefix #\: */
            if(mode)
            {
                s = ASTRING(o);
                printf(":%s",s->str);
                break;
            }
        case T_STRING:
            if(mode)
            {
                s = ASTRING(o);
                printf("\"%s\"",s->str);
                break;
            }
        case T_ATOM:
            s = ASTRING(o);
            printf("%s",s->str);
            break;
        case T_VECTOR:
            v = AVECTOR(o);
            printf("[");
            for(i = 0; i < v->len; i++)
            {
                princ(v->data[i],mode);
                if(i < (v->len - 1))
                    printf(" ");
            }
            printf("]");
            break;
        case T_ERROR:
        case T_VOID:
            if(mode)
                printf("#v");
            break;
    }
    return SVOID;
}

SExp
fprimadd(SExp a,SExp b)
/* there isn't any reason why this couldn't take
 * a variable number of arguments.
 */
{
    SExp ret, tmp0;
    Number *n0 = nil, *n1 = nil;
    if(!NUMBERP(a) || !NUMBERP(b))
        return makeerror(1,0,"add only operates on numbers");

    if(TYPE(b) > TYPE(a))
    {
        ret = b;
        tmp0 = a;
    }
    else
    {
        ret = a;
        tmp0 = b;
    }

    /* order types to minimize the number
     * of switches I need to have; rather than
     * having cases for each type combination,
     * reduce it so that we have
     *
     * c c
     * c q
     * c r
     * c i
     * q q
     * q r
     * q i
     * r r
     * r i
     * i i
     *
     * There are still quite a few cases, but their number
     * is reduced; doing it in graduating order also means
     * type promotion rules can be simplified.
     */
    if (TYPE(ret) == T_COMPLEX)
    {
        switch(TYPE(tmp0))
        {
            case T_INTEGER:
            case T_REAL:
            case T_RATIONAL:
            case T_COMPLEX:
                break;
        }
    }
    else if(TYPE(ret) == T_RATIONAL)
    {
        switch(TYPE(tmp0))
        {
            case T_RATIONAL:
            case T_REAL:
            case T_INTEGER:
                break;
        }
    }
    else if(TYPE(ret) == T_REAL)
    {
        n0 = hmalloc(sizeof(Number));
        switch(TYPE(tmp0))
        {
            case T_INTEGER:
            case T_REAL:
                break;
        }
    }
    else if(TYPE(ret) == T_INTEGER)
    {
        /* should be T_INTEGER only */
        ret = ((ret >> 32) + (tmp0 >> 32) << 32);

        SET_TYPE(ret,T_INTEGER);
    }
    return ret;
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
