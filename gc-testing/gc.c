#include <stdlib.h>

typedef struct GCO
{
    unsigned char mark;
    size_t length;
    void *ptr;
    struct GCO *next;
    struct GCO *prev;
} GCObject;

GCObject *init_gc_ring(int);
void *_alloc(size_t);
void gc();
void *gcalloc(size_t);
void mark(GCO);

typedef struct SEXP
{
    char c;
    unsigned int length;
    union
    {
        int i;
        double r;
        struct
        {
            double real;
            double imag;
        } c;
        struct
        {
            int num;
            int den;
        } q;
        char *str;
        struct SEXP **vec;
        struct {
            struct SEXP *car;
            struct SEXP *cdr;
        } pair;
    } object;
} SExp;   
