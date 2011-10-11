#include <stdio.h>
#include <gc.h>
#include <string.h>

#define TYPE(x) ((SExp)(x) & 0x1F)
#define SET_TYPE(x,type) ((SExp)(x) += (type) & 0x1F)
#define SET_INT(x,val) ((SExp)x) = (SExp)(val) << 32
#define SET_FLOAT(x,val) ((SExp)x = (SExp)*(&val) << 32)
#define SET_STRING(x,val) x = ((SExp)(val) << 32)
#define AINT(x) ((SExp)(x) >> 32)
#define AFLOAT(x) (float)((SExp)(x) >> 32)
#define GET_STRING(x) (String *)((SExp)(x) >> 32)

#define hmalloc GC_MALLOC

#define nil NULL

typedef long long SExp;

typedef enum {
    T_NULL = 0,T_INTEGER, T_RATIONAL, T_REAL, T_DREAL, T_COMPLEX,
    T_VOID, T_STRING, T_PAIR, T_ATOM, T_KEY, T_VECTOR,
    T_DICT, T_CLOSURE, T_PROCEDURE, T_FOREIGN, T_ERROR,
    T_CHAR, T_BOOL, T_GOAL
} SExpType;

typedef struct 
{
    int len;
    char *data;
} String;

int
main()
{
    GC_INIT();
    int val = 0;
    double f = 0.0f;
    char *str = hmalloc(sizeof(char) * 128);
    String *g = hmalloc(sizeof(String));
    String *h = nil;
    SExp sval = 0;
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("Please enter a number: ");
    scanf("%d",&val);
    SET_INT(sval,val);
    SET_TYPE(sval,T_INTEGER);
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("VALUE(x) == %d\n",AINT(sval));
    printf("Please enter a float: ");
    scanf("%f",&f);
    SET_FLOAT(sval,f);
    SET_TYPE(sval,T_REAL);
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("VALUE(x) == %f\n",AFLOAT(sval));
    fgets(str,128,stdin);
    printf("Please enter a string: ");
    str = fgets(str,128,stdin);
    g->data = str;
    g->len = strlen(str);
    printf("g->data == %s\n",g->data);
    printf("g->len == %d\n",g->len);
    printf("g == %x\n",g);
    printf("sval: %llx\n",sval);
    sval = 0l;
    SET_STRING(sval,g);
    SET_TYPE(sval,T_STRING);
    printf("sval: %llx\n",sval);
    h = GET_STRING(sval);
    printf("h == %x\n",h);
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("VALUE(x) == %s\n",h->data);
    printf("LEN(x) == %d\n",h->len);
    return 0;
}
