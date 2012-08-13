#include <stdlib.h>
#include <stdio.h>

#define nil NULL
#define nul '\0'

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
void mark(GCObject);

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

GCObject *
init_gc_ring(int len)
/* generate the initial ring of GCObjects
 * not sure if the final version will use
 * a list (AVL tree is my current thinking),
 * but it is good for prototype's sake.
 */
{
    GCObject *ret = nil, *tmp = nil;
    int idx = 0;
    ret = (GCObject *)malloc(sizeof(GCObject));
    tmp = ret;
    while(idx < (len - 1))
    {
        if(!tmp)
            return nil;
        tmp->next = (GCObject *)malloc(sizeof(GCObject));
        tmp->next->prev = tmp;
        tmp = tmp->next;
        idx ++;
    }
    tmp->next = nil;
    return ret;
}

void *
gcalloc(size_t sze)
{
    void *ret = nil, *tmp = nil;
    GCObject item = nil;
    if(lptr->next == nil)
    {
        /* lptr points to the last
         * item in the free list. Attempt
         * a GC to clear it.
         */  
        gc(); // should this update lptr, or what?
        if(gc_somehow_failed)
        {
            tmp = init_gc_ring(20);
            if(tmp == nil)
            {
                printf("unable to malloc more memory! exiting\n");
                return nil;
            }
            lptr->next = tmp;
        }
    }
    ret = (void *)malloc(sze);
    item = lptr->next;
    item->ptr = ret;
    item->mark = mark_direction;
    item->length = sze;
    lptr = lptr->next;
    return ret;
}
