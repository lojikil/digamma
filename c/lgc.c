#include <stdio.h>
#include <stdlib.h>
#include "vesta.h"

/* limited garbage collection; a quick & dirty solution to some
 * compilations I'm currently working through with Boehm.
 */

GCObject *
init_gc_ring(int len, Symbol *env)
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
    tmp->mark = env->mark_direction;
    while(idx < (len - 1))
    {
        if(!tmp)
            return nil;
        tmp->next = (GCObject *)malloc(sizeof(GCObject));
        tmp->next->prev = tmp;
        tmp->next->mark = env->mark_direction;
        tmp = tmp->next;
        idx ++;
    }
    tmp->next = nil;
    return ret;
}

void *
gcalloc(size_t sze, Symbol *env)
{
    void *ret = nil, *tmp = nil;
    GCObject item = nil;
    if(lptr->next == nil)
    {
        /* lptr points to the last
         * item in the free list. Attempt
         * a GC to clear it.
         */  
        gc(); 
        if(lptr->next == nil)
        {
            tmp = init_gc_ring(20, env);
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
    item->mark = env->mark_direction;
    item->length = sze;
    lptr = lptr->next;
    return ret;
}

void
gc(Symbol *env)
{
    
}

void
mark_sexpr(SExp *object)
{

}

void
mark_literal(void *)
{
    /* used for strings/keywords/atoms */
}
