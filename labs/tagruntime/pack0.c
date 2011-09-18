#include <stdio.h>

#define TYPE(x) ((SExp)(x) & 0x1F)
#define SET_TYPE(x,type) ((SExp)(x) += (type) & 0x1F)
#define SET_INT(x,val) ((SExp)x) += (SExp)(val) << 32
#define SET_FLOAT(x,val) ((SExp)x) += (SExp)(val) << 32
#define AINT(x) ((SExp)(x) >> 32)
#define AFLOAT(x) (float)((SExp)(x) >> 32)

typedef long long SExp;

typedef enum {
    T_NULL = 0,T_INTEGER, T_RATIONAL, T_REAL, T_DREAL, T_COMPLEX,
    T_VOID, T_STRING, T_PAIR, T_ATOM, T_KEY, T_VECTOR,
    T_DICT, T_CLOSURE, T_PROCEDURE, T_FOREIGN, T_ERROR,
    T_CHAR, T_BOOL, T_GOAL
} SExpType;

int
main()
{
    int val = 0;
    float f = 0.0f;
    SExp sval = 0;
    printf("TYPE(x) == %d\n",TYPE(sval));
    SET_TYPE(sval,T_INTEGER);
    printf("Please enter a number: ");
    scanf("%d",&val);
    SET_INT(sval,val);
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("VALUE(x) == %d\n",AINT(sval));
    printf("Please enter a float: ");
    scanf("%f",&f);
    SET_TYPE(sval,T_REAL);
    SET_FLOAT(sval,f);
    printf("TYPE(x) == %d\n",TYPE(sval));
    printf("VALUE(x) == %f\n",AFLOAT(sval));
    return 0;
}
