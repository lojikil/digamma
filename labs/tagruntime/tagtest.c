/* Simple tagged runtime test
 * Copyright 2011, Stefan Edwards, released under
 * the zlib/png license. See the LICENSE file
 * for details
 */

#include <stdio.h>
#include <gc.h>

#define nil NULL

#define TYPE(x) ((x) & 0x1F)
#define SET_TYPE(o,x) ((o) + (0xc0 + x)) 
#define NUMBERP(x) (TYPE(x) == T_INTEGER || TYPE(x) == T_RATIONAL\
                    TYPE(X) == T_REAL || TYPE(x) == T_DREAL \
                    TYPE(X) == T_COMPLEX)

#define AINT(x) 0
#define AREAL(x) 0
#define ANUM(x) 0
#define ADEN(x) 0
#define AIMAG(x) 0
#define ACEREAL(x) 0
#define ANUMBER(x) 0
#define APAIR(x) 0
#define ASTRING(x) 0
#define ABOOL(x) 0
#define AGOAL(x) 0

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

SExp makeinteger(int);
SExp makereal(double);
SExp makecomplex(double, double);
SExp makerational(int,int);
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
    GET_PTR(f,n0);
    if(n0->d != 1.0)
        printf("real-value is not == 1.0\n");
    else
        printf("pass real value check\n");

    g = makerational(3,4);
    if(TYPE(g) != T_RATIONAL)
        printf("incorrect type set for rationals\n");
    else
        printf("pass rational type check\n");
    GET_NUM(g,n0);
    if(n0->n != 3 || n0->d != 4)
        printf("rational-value is not == 3/4\n");
    else
        printf("pass rational value check\n");
     
    f = makecomplex(1.0,0.75);
    if(TYPE(f) != T_COMPLEX)
        printf("incorrect type set for complex numbers\n");
    else
        printf("pass complex type check\n");
    GET_NUM(f,n0);
    if(AREAL(n0) != 1.0 || AIMAG(n0) != 0.75)
        printf("incorrect value for complex check\n");
    else
        printf("pass complex value check\n");
    
    printf("BASIC Numerical operations checks\n");

    f = fprimadd(makeinteger(1),makeinteger(1));
    if(AINT(f) != 2)
        printf("primitive addition fails for integer\n");
    else
        printf("pass primitive integer addition\n");

    f = fprimadd(makeinteger(1),g);
    GET_NUM(f,n0);
    if(TYPE(f) != T_RATIONAL)
        printf("primitive addition fails integer + rational type check\n");
    else if(n0->num != 7 || n0->den != 4)
        printf("primitive addtion fails integer + rational value check\n");
    else
        printf("pass primitive addition integer + rational\n");

    /* these two should be the same, but check that primitive addition
     * handles these cases correctly
     */
    f = fprimadd(g,makeinteger(1));
    GET_NUM(f,n0);
    if(TYPE(f) != T_RATIONAL)
        printf("primitive addition fails rational + integer type check\n");
    else if(n0->num != 7 || n0->den != 4)
        printf("primitive addtion fails rational + integer value check\n");
    else
        printf("pass primitive addition rational + integer\n");

    f = primadd(g,makereal(1.0));
    GET_NUM(f,n0);
    if(TYPE(f) != T_REAL)
        printf("primitive addition fails for rational + real type check\n");
    else if(n0->d != 1.75)
        printf("primitive addition fails for rational + real value check\n");
    else
        printf("pass primitive addition rational + real\n");

    f = primadd(g,makecomplex(1.0,0.75));

    f = makestring("this is a test");

    f = cons(f,cons(g,SNULL));
}
#endif 

SExp
makeinteger(int i)
{
    SExp ret;
    SET_INT(ret,i);
    SET_TYPE(ret,T_INTEGER);
    return ret;
}

SExp
makereal(double d)
{
    SExp ret;
    Number *r = hmalloc(sizeof(Number));
    r->d = d;
    SET_TYPE(ret,T_REAL);
    SET_PTR(ret,r);
    return ret;
}

SExp
makerational(int n, int d)
{
    SExp ret;
    Number *q = hmalloc(sizeof(Number));
    q->num = n;
    q->den = d;
    SET_TYPE(ret,T_RATIONAL);
    SET_PTR(ret,q);
    return ret;
}

SExp 
makecomplex(double real, double imag)
{
    SExp ret;
    Number *c = hmalloc(sizeof(Number));
    c->real = real;
    c->imag = imag;
    SET_TYPE(ret,T_COMPLEX);
    SET_PTR(ret,q);
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
    Pair *p = hmalloc(sizeof(Pair));
    p->car = a;
    p->cdr = b;
    SET_PTR(ret,p);
    return ret;
}

SExp
car(SExp o)
{
    Pair *p = nil;
    if(TYPEP(o,T_PAIR))
    {
        GET_PTR(o,p);
        return p->car;
    }
    return SNULL;
}

SExp
cdr(SExp o)
{
    Pair *p = nil;
    if(TYPEP(o,T_PAIR))
    {
        GET_PTR(o,p);
        return p->cdr;
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
            printf("%f",AREAL(d));
            break;
        case T_RATIONAL:
            Number *q = ANUMBER(o);
            printf("%d/%d",ANUM(q),ADEN(q));
            break;
        case T_COMPLEX:
            Number *c = ANUMBER(o);
            printf("%f+%fi",ACEREAL(c),AIMAG(c));
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
                printf(":");
        case T_STRING:
            if(mode)
            {
                String *s = ASTRING(o);
                printf("\"%s\"",s);
                break;
            }
        case T_ATOM:
            String *s = ASTRING(o);
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
    Number *a
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
