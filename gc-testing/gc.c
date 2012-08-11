#include <stdlib.h>

/* the basic outline of the GCObject. Used for
 * the allocation list. Instead of maintaining
 * a free list, keep *one* single list, and change
 * a "direction" bit that represents if something
 * is free/used. Basically, mark-don't-sweep
 */

typedef struct GCO
{
    unsigned char mark;
    size_t length;
    void *ptr;
    struct GCO *next;
    struct GCO *prev;
} GCObject;

/* the code functions of the GC system.
 * - init_gc_ring initializes the initial allocation
 * node list. 
 * - gc kicks off the Garbage collection process
 * - gcalloc is the drop in replacement for malloc
 *   (though, not really, since it will most likely
 *    have to take an environment parameter too)
 * - mark strolls through the environment & checks
 * out what can be marked as free.
 */

GCObject *init_gc_ring(int);
void gc();
void *gcalloc(size_t);
void mark(GCO);

/* global GC objects. in the real system these
 * would be stored in the Symbol object, so as
 * to support multiple-potential GC 
 * environments.
 */

static GCObject *head; // the head of the allocation list
static GCObject *lptr; // pointer to the last node allocated
static unsigned char mark_direction; // 1 || 0

typedef enum 
{
    CHAR,
    BOOL,
    GOAL,
    STRING,
    ATOM,
    KEY,
    VECTOR,
    PAIR,
    INTEGER,
    REAL,
    RATIONAL,
    COMPLEX
} SExpType;

/* an approximation of what Vesta's built in giant
 * object looks like.
 */

typedef struct SEXP
{
    char c;
    SExpType type;
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
