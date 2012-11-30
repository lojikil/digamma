#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gc.h>
#include <math.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include "vesta.h"

extern const char *typenames[];
extern int quit_note;

SExp *
__seval(SExp *s, Symbol *e)
{
    SExp *src = s, *fst = e->snil, *rst = e->snil, *tmp0 = e->snil, *tmp1 = e->snil,*tmp3 = e->snil;
    SExp *tmp2 = e->snil, *stk = e->snil, *__r = e->snil, *__val = e->snil, *ret = e->snil;
	Symbol *tenv = nil, *env = e;
	int state = __PRE_APPLY, itmp = 0,tail = 0; /* tail is a flag for __PRE_APPLY & OPBEGIN */
	SExp *(*proc)(SExp *, Symbol *) = nil;
	if(s == nil)
		return e->svoid;
	env = shallow_clone_env(e);
__base:
#ifdef DEBUG
	printf("Stack depth: %d; State: %d\n",pairlength(stk),state);
    printf("Env depth: %d; Total: %d\n",env->cur_offset, env->cur_size);
	printf("src == ");
	princ(src);
	printf("\n");
#endif
	// add interpreter tick here.
	switch(state)
	{
		case __PRE_APPLY:
            LINE_DEBUG;
			e->tick++;
			if(src->type == PAIR)
			{
				/*printf("ret(0) == ");
				princ(ret);
				printf("\n");
				if(ret != e->snil && mcar(ret) != e->snil)
					fst = mcar(mcar(ret));
				else*/
				fst = car(src);
				/*printf("fst == ");
				princ(fst);
				printf("\n");*/
				rst = cdr(src);
				ret = tconcify(e->snil);
				/*printf("rst == ");
				princ(rst);
				printf("\nret(1) == ");
				princ(ret);
				printf("\n");*/
				if(fst->type == ATOM)
				{
					tmp2 = fst;
					//printf("fst == %s\n",fst->object.str);
					fst = symlookup(fst->object.str,env);
					if(fst == nil)
					{
						printf("\n\nSrc == (%s)\n",typenames[src->type]);
						princ(src);
						printf("\n");
						printf("Symbol: %s\n",tmp2->object.str);
						__return(makeerror(1,0,"unknown symbol"));
					}
				}
				if(fst->type == PAIR)
				{
					// jump schuck
					/*printf("Made it to fst->type == PAIR\n");
					stk = cons(vector(6,src,fst,rst,ret,env->data,makeinteger(state)),stk);
					src = fst;
					state = __PRE_APPLY;
					goto __base;*/
					// turn this into jump shuck momentarily
					//fst = __seval(fst,e); 
					fst = __seval(fst,env); 
				}
				if(fst->type == SYNTAX)
				{
					src = syntax_expand(src,env);
					if(src->type == PAIR)
						goto __base;
					else if(src->type == ATOM)
					{
						tmp1 = symlookup(src->object.str,env);
						if(tmp1 == nil)
						{
							__return(makeerror(1,0,"unknown symbol"));
						}
						__return(tmp1);
					}
					else 
					{
						__return(src);
					}
				}
				if(fst->type == MACRO)
				{
					src = macro_expand(src,env);
					if(src->type == PAIR)
						goto __base;
					else if(src->type == ATOM)
					{
						tmp1 = symlookup(src->object.str,env);
						if(tmp1 == nil)
						{
							__return(makeerror(1,0,"unknown symbol"));
						}
						__return(tmp1);
					}
					else 
					{
						__return(src);
					}
				}
				if(fst->type == PRIM && fst->object.primitive.evalp)
				{
					state = fst->object.primitive.num;
					goto __base;
				}
				if(fst->type != PRIM && fst->type != PROCEDURE &&
                   fst->type != VECTOR && fst->type != STRING &&
                   fst->type != DICT && fst->type != CLOSURE &&
                   fst->type != CONTINUATION)
				{
					__return(makeerror(1,0,"invalid type for application: only primitives, procedures, continuations, vectors, strings & dictionaries may be applied"));
				}
			}
			else
			{
                if(src->type == ATOM)
                {
                    tmp1 = symlookup(src->object.str, env);
                    if(tmp1 == nil)
                    {
                        __return(makeerror(1,0,"unknown symbol in __seval"));
                    }
                    __return(tmp1);
                }
				__return(src);
			}
			//goto __base;
		case __POST_APPLY:
            LINE_DEBUG;
			tmp0 = rst;
			/* it would be great if we could unify closure arguments
			 * whilst processing args for application, but for now
			 * I'll keep the two separate
			 */
			while(tmp0 != e->snil)
			{
                LINE_DEBUG;
				tmp1 = car(tmp0);
				tmp0 = cdr(tmp0);
				if(tmp1->type == PAIR)
				{
					// jump schuck
					stk = cons(vector(6,src,fst,tmp0,ret,env->data,makeinteger(__POST_APPLY)),stk);
					src = tmp1;
					state = __PRE_APPLY;
					goto __base;
				}
				else if(tmp1->type == ATOM)
				{
					// symlookup
					tmp2 = tmp1;
					tmp1 = symlookup(tmp1->object.str,env);
					if(tmp1 == nil)
					{
						printf("rst == ");
						princ(rst);
						printf("\n");
						printf("%s\n",tmp2->object.str);
						__return(makeerror(1,0,"unknown symbol in __POST_APPLY lookup"));
					}
					tconc(ret,tmp1);
				}
				else
				{
					tconc(ret,tmp1);
				}
			}
			rst = mcar(ret);
        case __POST_POST_APPLY:
            LINE_DEBUG;
			switch(fst->type)
			{
				case PRIM:
					state = fst->object.primitive.num;
					break;
				case PROCEDURE:
					state = __PROC;
					break;
                case CONTINUATION:
                    state = __CONT;
                    break;
				case CLOSURE:
                    LINE_DEBUG;
					// unify formal parameters with arguments
					// Need to add a check here for a tail call condition.
                    LINE_DEBUG;
					env->data = (Window *) fst->object.closure.env;
					tmp0 = fst->object.closure.params;
					/* add test for tail here... */
					if(rst == e->snil)
					{
						if(tmp0->type != NIL && mcar(tmp0)->type != KEY)
						{
							// attempt a stack trace
                            // TODO: check if this is actually needed here 
							printf("Stack trace: \n");
							tmp1 = stk;
							while(tmp1 != e->snil)
							{
								tmp2 = car(car(tmp1));
								llprinc(tmp2,stdout,1);
								printf("\n");
								tmp1 = cdr(tmp1);
							}
							__return(makeerror(1,0,"Unsatisfied arguments to procedure with non-optional parameters"));
						}
					}
					/* process formal parameters 
					 * also, if tail == 0 (i.e. not a tail call)
					 * create a new window; otherwise, modify in-place
					 * and set tail = 0;
					 */
					/*if(!tail)
						new_window(env);
					else
						tail = 0;*/
					new_window(env);
					while(tmp0 != e->snil)
					{
						tmp1 = car(tmp0);
						if(tmp1->type == ATOM)
						{
							if(rst == e->snil)
							{
                                printf("source of unsatisfied procedure argument: ");
                                llprinc(src, stdout, 1);
                                printf("\n");
                                printf("argument: %s\n",tmp1->object.str);
								__return(makeerror(1,0,"Unsatisfied non-optional procedure argument"));
							}
							add_env(env,tmp1->object.str,car(rst));
						}
                        else if(tmp1->type == PAIR)
						{
                            tmp2 = tmp1;
							tmp3 = car(tmp2);
							tmp2 = car(cdr(tmp2));
							if(rst != e->snil)
							    add_env(env,tmp3->object.str,car(rst));
							else
							{
							    tmp2 = __seval(tmp2,env);
						        add_env(env,tmp3->object.str,tmp2);
							}
                        }
						tmp0 = cdr(tmp0);
                        if(tmp0->type == ATOM)
                        {
                            add_env(env,tmp0->object.str,rst);
                            break;
                        }
						rst = cdr(rst);
					}
					rst = fst->object.closure.data;
					state = OPBEGIN;
					break;
			}
			goto __base;
		case OPCAR:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"car list : PAIR => S-EXPRESSION"));
			}
			__return(car(car(rst)));
		case OPCDR:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"cdr list : PAIR => S-EXPRESSION"));
			}
			__return(cdr(car(rst)));
		case OPCONS:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"cons s0 : S-EXPRESSION s1 : S-EXPRESSION => S-EXPRESSION"));
			}
			__return(cons(car(rst),car(cdr(rst))));
		case OPLAMBDA:
			__return(ffn(rst,env));
		case OPDEF:
			__return(fdef(car(rst),rst,env)); // do we really need to __return? I guess if you want the #v...
		case OPLENGTH:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"length s : S-EXPRESSION => NUMBER?"));
			}
			__return(flength(car(rst)));
		case OPDEFMACRO:
			if(pairlength(rst) < 3)
			{
				__return(makeerror(1,0,"define-macro (name : SYMBOL) (bindings*) (form*) => macro"));
			}
			__return(fdefmacro(car(rst),car(cdr(rst)),cdr(cdr(rst)),env));
		case OPDEFSYN:
			if(pairlength(rst) < 2)
			{
				__return(makeerror(1,0,"define-syntax (name : SYMBOL) (rewrite-rule*) => syntax"));
			}
			__return(fdefsyntax(car(rst),cdr(rst),env));
		case OPQUOTE:
			__return(car(rst));
		case OPPLUS:
			__return(fplus(rst));
		case OPMULT:
			__return(fmult(rst));
		case OPSUB:
			__return(fsubt(rst));
		case OPDIV:
			__return(fdivd(rst));
		case OPLIST:
			__return(rst);
		case OPVECTOR:
			__return(fvector(rst));
		case OPDICT:
			__return(fdict(rst));
		case OPMKSTRING:
			__return(fmakestring(rst));
		case OPMKVEC:
			__return(fmkvector(rst)); // inconsistent; rename to fmakevector
		case OPMKDICT:
			__return(makedict());
		case OPEVAL:
			itmp = pairlength(rst);
			if(itmp != 1 && itmp != 2)
			{
				__return(makeerror(1,0,"eval f : FORM [e : ENVIRONMENT] => S-EXPRESSION"));
			}
			if(itmp == 1)
			{
				__return(__seval(car(rst),env));
			}
			else
			{
				tmp0 = car(cdr(rst));
				if(tmp0->type == ATOM)
				{
					tmp0 = symlookup(tmp0->object.str,env);
					if(tmp0 == nil)
					{
						__return(makeerror(1,0,"unbound symbol for eval's environment parameter"));
					}
				}
				else if(tmp0->type == PAIR)
				{
					tmp0 = __seval(tmp0,env);
				}
				if(tmp0->type != ENVIRONMENT)
				{
					__return(makeerror(1,0,"eval's second parameter *must* be of type environment"));
				}
				__return(__seval(car(rst),(Symbol *)tmp0->object.foreign));
			}
			break;
		case OPAPPLY:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"apply a : APPLIABLE l : PAIR => S-EXPRESSION"));
			}
			//src = cons(car(rst),car(cdr(rst)));
            fst = car(rst);
            rst = car(cdr(rst));
			state = __POST_POST_APPLY;
			goto __base;
		case OPSTRING:
			__return(fstring(rst));
		case OPCCONS: /* collection cons... */
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"ccons (s : S-EXPRESSION) (c : COLLECTION)"));
			}
			__return(fccons(car(rst),car(cdr(rst))));
		case OPFIRST:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"first expects only one argument, a collection..."));
			}
			__return(ffirst(car(rst)));
		case OPREST:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"rest (c : COLLECTION) => s-expression"));
			}
			__return(frest(car(rst)));
		case OPNTH:
            itmp = pairlength(rst);
			if(itmp == 2)
			{
			    __return(fnth(car(rst),car(cdr(rst)),nil));
            }
            else if(itmp == 3)
            {
			    __return(fnth(car(rst),car(cdr(rst)),car(cdr(cdr(rst)))));
            }
			__return(makeerror(1,0,"nth (c : COLLECTION) (idx: INTEGER) [default: S-EXPRESSION] => s-expression"));
		case OPKEYS:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"keys (c : COLLECION) => PAIR"));
			}
			tmp0 = car(rst);
			if(tmp0->type != DICT)
			{
				__return(makeerror(1,0,"Keys currently only operates on dictionaries"));
			}
			__return(fkeys(tmp0));
		case OPPARTIAL: // partial-key?
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"partial-key? only accepts two arguments"));
			}
			tmp0 = car(rst);
			tmp1 = car(cdr(rst));
			if(tmp0->type != DICT)
			{
				__return(makeerror(1,0,"partial-key? only operates on tries"));
			}
			if(tmp1->type != STRING && tmp1->type != ATOM && tmp1->type != KEY)
			{
				__return(makeerror(1,0,"partial-key? only shards on (ATOM | STRING | KEYOBJ)"));
			}
			__return(fpartial_key(tmp0,tmp1));
		case OPCSET:
			if(pairlength(rst) != 3)
			{
				__return(makeerror(1,0,"cset! (c : COLLECTION) (idx : SEXPRESSION) (new : SEXPRESSION) => void?"));
			}
			__return(fcset(car(rst),car(cdr(rst)),car(cdr(cdr(rst)))));
		case OPCUPDATE:
			if(pairlength(rst) != 3)
			{
				__return(makeerror(1,0,"cupdate col : COLLECTION idx : SEXPRESSION new-value : SEXPRESSION => COLLECTION"));
			}
			__return(fcupdate(car(rst),car(cdr(rst)), car(cdr(cdr(rst)))));
		case OPCSLICE:
			if(pairlength(rst) != 3)
			{
				__return(makeerror(1,0,"cslice col : COLLECTION start : SEXPRESSION end : SEXPRESSION => COLLECTION"));
			}
			__return(fcslice(car(rst),car(cdr(rst)), car(cdr(cdr(rst)))));
		case OPEMPTY: /* generic empty predicate */
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"empty? expects exactly one argument..."));
			}
			__return(fempty(car(rst)));
		case OPSET:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"set! expects exactly two arguments..."));
			}
			__return(fset(car(rst),car(cdr(rst)),env));
		case OPGENSYM:
			if(pairlength(rst) > 1)
			{
				__return(makeerror(1,0,"gensym [(a : ATOM)] => symbol"));
			}
			__return(fgensym(car(rst)));
		case OPAPPEND: 
			__return(fappend(rst));	
		case OPTYPE:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"type expects exactly one argument..."));
			}
			__return(makestring((char *)typenames[mcar(rst)->type]));
		case OPEQ:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"eq? expects two arguments..."));
			}
			__return(eqp(car(rst), car(cdr(rst))));
		case OPCALLCC:
            if(pairlength(rst) != 1)
            {
                __return(makeerror(1,0,"call/cc expects a single, applicable, argument"));
            }
            tmp0 = car(rst);
            tmp2 = (SExp *)hmalloc(sizeof(SExp));
            tmp2->type = CONTINUATION;
#ifdef DEBUG
            printf("\nstk dump\n");
            tmp3 = stk;
            while(tmp3 != e->snil && tmp3 != nil)
            {
                printf("tmp3: ");
                tmp1 = car(tmp3);
                tmp3 = cdr(tmp3);
                for(itmp = 0;itmp < 6; itmp++)
                {
                    if(itmp != 4)
                    {
                        printf("\t%d: ", itmp);
                        princ(tmp1->object.vec[itmp]);
                        printf("\n");
                    }
                    else
                        printf("\tenv->data\n");
                }
                printf("\n");
            }
            printf("current ret: ");
            princ(ret);
            printf("\n");
            printf("__val: ");
            princ(__val);
            printf("\n");
            printf("__r: ");
            //princ(__r);
            printf("\n");
#endif
            //stk = cdr(stk);
            tmp2->object.closure.data = stk;
            state = __POST_POST_APPLY;
            src = cons(tmp0,cons(tmp2,e->snil));
            rst = cons(tmp2,e->snil);
            fst = tmp0;
            goto __base; 
		case OPLT: /* < */
			__return(flt(rst));
		case OPLTE:
			__return(flte(rst));
		case OPGT:
			__return(fgt(rst));
		case OPGTE:
			__return(fgte(rst));
		case OPNUMEQ:
			__return(fnumeq(rst));
		case OPIF:
            itmp = pairlength(rst);
            if(itmp < 2 || itmp > 3)
            {
                __return(makeerror(1,0,"if <COND> <THEN> [<ELSE>]"));
            }
            tmp0 = __seval(car(rst),env);
            if(tmp0 == nil)
                return env->snil;
            if(tmp0->type == ERROR)
               return tmp0;
            if((tmp0->type == GOAL || tmp0->type == BOOL) && tmp0->object.c) /* e.g. is it false */
            {
                tmp1 = car(cdr(rst));
                switch(tmp1->type)
                {
                    case PAIR:
                        src = tmp1;
                        state = __PRE_APPLY;
                        goto __base;
                    case ATOM:
                        tmp1 = symlookup(tmp1->object.str,env);
                        if(!tmp1)
                        {
                            __return(makeerror(1,0,"Unknown symbol in if's <THEN> branch"));
                        }
                    default:
                        __return(tmp1);
                }
            }
            /* an if block with no <ELSE> and a false <COND> */
            if(itmp == 2)
            {
                __return(env->sfalse);
            }
            tmp1 = car(cdr(cdr(rst)));
            switch(tmp1->type)
            {
                case PAIR:
                    src = tmp1;
                    state = __PRE_APPLY;
                    goto __base;
                case ATOM:
                    tmp1 = symlookup(tmp1->object.str,env);
                    if(!tmp1)
                    {
                        __return(makeerror(1,0,"Unknown symbol in if's <ELSE> branch"));
                    }
                default:
                    __return(tmp1);
            }
		case OPUNWIND:
			__return(e->snil);
		case OPEXACT:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"exact? expects only on argument..."));
			}
			__return(fexactp(car(rst)));
		case OPINEXACT:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"inexact? expects only on argument..."));
			}
			__return(finexactp(car(rst)));
		case OPREAL:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"real? expects only on argument..."));
			}
			__return(frealp(car(rst)));
		case OPCOMPLEX:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"complex? expects only on argument..."));
			}
			__return(fcomplexp(car(rst)));
		case OPRATIONAL:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"rational? expects only on argument..."));
			}
			__return(frationalp(car(rst)));
		case OPINTEGER:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"integer? expects only on argument..."));
			}
			__return(fintegerp(car(rst)));
		case OPBEGIN:
			/*printf("MADE IT TO OPBEGIN\n");
			printf("RST == ");
			princ(rst);
			printf("\nfst ==");
			princ(fst);
			printf("\nsrc == ");*/
			tmp0 = rst;
			/*princ(src);
			printf("\n");*/
			tmp1 = car(tmp0);
			/*printf("tmp1 == ");
			princ(tmp1);
			printf("\ntmp0 == ");
			princ(tmp0);
			printf("\n");*/
			tmp0 = cdr(tmp0);
			//printf("Looping?\n");
			if(tmp1->type == PAIR)
			{
				// state == OPBEGIN
				if(tmp0 == e->snil)
				{
					/* Ok, so what we're doing here:
					 * if it's a tail call in a closure,
					 * and we're calling the same procedure,
					 * set tail = 1, so that we modify parameters in place
					 * otherwise, just use a normal window
					 */
                    if(fst->type == CLOSURE)
					    tail = 1;
				}
				else
					stk = cons(vector(6,src,fst,tmp0,ret,env->data,makeinteger(state)),stk);
				src = tmp1;
				state = __PRE_APPLY;
				//printf("Pushing?\n");
				goto __base;
			}
			else if(tmp1->type == ATOM)
			{
				tmp2 = tmp1;
				tmp1 = symlookup(tmp1->object.str,env);
				if(tmp1 == nil)
				{
					printf("symbol: %s",tmp2->object.str);
					__return(makeerror(1,0,"unknown symbol"));
				}
                if(tmp0 == e->snil)
				    tconc(ret,tmp1);
                else
                {
                    tmp1 = car(tmp0);
                    tmp0 = cdr(tmp0);
                    if(tmp0 != e->snil)
                        stk = cons(vector(6,src,fst,tmp0,ret,env->data,makeinteger(state)),stk);
                    src = tmp1;
                    state = __PRE_APPLY;
                    goto __base;
                }
			}
			else
            {
                if(tmp0 == e->snil)
				    tconc(ret,tmp1);
                else
                {
                    tmp1 = car(tmp0);
                    tmp0 = cdr(tmp0);
                    if(tmp0 != e->snil)
                        stk = cons(vector(6,src,fst,tmp0,ret,env->data,makeinteger(state)),stk);
                    src = tmp1;
                    state = __PRE_APPLY;
                    goto __base;
                }
            }
		    if(fst->type == CLOSURE)
			{
				//close_window(env);
				//env->data = (Window *)__r->object.vec[4];
			}
			/*printf("\n\n\nMade it to the end of OPBEGIN; returning :");
			princ(ret);
			printf("\n");*/
			__return(mcar(mcdr(ret)));
		case OPNUM: /* numerator */
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"numerator expects one argument..."));
			}
			__return(fnum(car(rst)));
		case OPDEN: /* denomenator */
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"denomenator expects one argument..."));
			}
			__return(fden(car(rst)));
		case OPAND:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"& expects exactly two integer arguments"));
			}
			__return(fbitand(car(rst),car(cdr(rst))));
		case OPOR:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"| expects exactly two integer arguments"));
			}
			__return(fbitor(car(rst),car(cdr(rst))));
		case OPXOR:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"& expects exactly two integer arguments"));
			}
			__return(fbitxor(car(rst),car(cdr(rst))));
		case OPNEG:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"~ expects exactly one integer arguments"));
			}
			__return(fbitnot(car(rst)));
		case OPSHL:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"<< expects exactly two integer arguments"));
			}
			__return(fbitshl(car(rst),car(cdr(rst))));
		case OPSHR:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,">> expects exactly two integer arguments"));
			}
			__return(fbitshr(car(rst),car(cdr(rst))));
		case OPREALP:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"real-part expects exactly one argument..."));
			}
			__return(freal_part(car(rst)));
		case OPIMAG:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"imag-part expects exactly one argument..."));
			}
			__return(fimag_part(car(rst)));
		case OPMKRECT:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"make-rectangular expects two arguments only"));
			}
			__return(fmake_rect(car(rst),car(cdr(rst))));
		case OPMKPOL:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"make-polar expects two arguments only"));
			}
			__return(fmake_pole(car(rst), car(cdr(rst))));
		case OPCONJ:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"conjugate accepts only one argument (a complex number)"));
			}
			__return(fconjugate(car(rst)));
		case OPCONJBANG:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"conjugate! accepts only one argument (a complex number)"));
			}
			tmp0 = car(rst);
			__return(fconjugate_bang(car(rst)));
		case OPPOLREC: /* polar->rectangular */
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"polar->rectangular accepts only one argument, a complex number"));
			}
			__return(fpol2rect(car(rst)));
		case OPRECPOL: /* rectangular->polar */
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"rectangular->polar accepts only one argument, a complex number"));
			}
			__return(frect2pol(car(rst)));
		case OPGCD:
			__return(fgcd(rst));
		case OPLCM:
			__return(flcm(rst));
		case OPQUOTIENT: 
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"quotient (x0 : NUMBER) (x1 : NUMBER) => NUMBER"));
			}
			__return(fquotient(car(rst),car(cdr(rst))));
		case OPMOD:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"modulo (x0 : NUMBER) (x1 : NUMBER) => NUMBER"));
			}
			__return(fmodulo(car(rst),car(cdr(rst))));
		case OPREMAINDER:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"remainder (x0 : NUMBER) (x1 : NUMBER) => NUMBER"));
			}
			__return(fremainder(car(rst),car(cdr(rst))));
		case OPSIN:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"sin expects exactly one argument..."));
			}
			__return(fsin(car(rst)));
		case OPCOS:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"cos expects exactly one argument..."));
			}
			__return(fcos(car(rst)));
		case OPTAN:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"tan expects exactly one argument..."));
			}
			__return(ftan(car(rst)));
		case OPASIN:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"asin expects exactly one argument..."));
			}
			__return(fasin(car(rst)));
		case OPACOS:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"acos expects exactly one argument..."));
			}
			__return(facos(car(rst)));
		case OPATAN:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"atan expects exactly one argument..."));
			}
			__return(fatan(car(rst)));
		case OPATAN2:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"atan2 expects exactly two arguments..."));
			}
			__return(fatan2(car(rst),car(cdr(rst))));
		case OPCOSH:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"cosh expects exactly one argument..."));
			}
			__return(fcosh(car(rst)));
		case OPSINH:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"sinh expects exactly one argument..."));
			}
			__return(fsinh(car(rst)));
		case OPTANH:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"tanh expects exactly one argument..."));
			}
			__return(ftanh(car(rst)));
		case OPEXP:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"exp expects exactly one argument..."));
			}
			__return(fexp(car(rst)));
		case OPEXP2:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"exp2 expects exactly one argument..."));
			}
			__return(fexp2(car(rst)));
		case OPEXPM1:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"expm1 expects exactly one argument..."));
			}
			__return(fexpm1(car(rst)));
		case OPLN:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"ln expects exactly one argument..."));
			}
			__return(fln(car(rst)));
		case OPLOG2:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"log2 expects exactly one argument..."));
			}
			__return(flog2(car(rst)));
		case OPLOG10:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"log10 expects exactly one argument..."));
			}
			__return(flog10(car(rst)));
		case OPABS:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"abs expects exactly one argument..."));
			}
			__return(fnabs(car(rst)));
		case OPMAG:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"magnitude expects exactly one argument..."));
			}
			__return(fmag(car(rst)));
		case OPSQRT:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"sqrt expects exactly one argument..."));
			}
			__return(fsqrt(car(rst)));
		case OPSTRAP: /* string-append */
			__return(fstringappend(rst));
		case OPASSQ:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"assq item : EQABLE-SEXPRESSION alist : ASSOC-LIST => sexpression"));
			}
			tmp0 = car(rst);
			tmp1 = car(cdr(rst));
			if(tmp1->type != PAIR && tmp1->type != NIL)
			{
				__return(makeerror(1,0,"alist must be an ASSOC-LIST (an hence a pair)"));
			}
			__return(assq(tmp0,tmp1));
		case OPMEMQ:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"memq item : EQABLE-SEXPRESSION member-list : PAIR => (PAIR | FALSE)"));
			}
			tmp0 = car(rst);
			tmp1 = car(cdr(rst));
			if(tmp1->type != PAIR && tmp1->type != NIL)
			{
				__return(makeerror(1,0,"member-list must be a pair"));
			}
			__return(memq(tmp0,tmp1));
		case OPDICHAS:
			/* simple key test that returns true or false, and does not signal an error */
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"dict-has? d : DICTIONARY k : (KEYOBJ | STRING | ATOM) => S-EXPRESSION"));
			}
			__return(fdicthas(car(rst),car(cdr(rst))));
		case OPCEIL:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"ceil r : REAL => REAL"));
			}
			__return(fceil(car(rst)));
		case OPFLOOR:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"floor r : REAL => REAL"));
			}
			__return(ffloor(car(rst)));
		case OPTRUNCATE:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"truncate r : REAL => INTEGER"));
			}
			__return(ftrunc(car(rst)));
		case OPROUND:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"round r : REAL => INTEGER"));
			}	
			__return(fround(car(rst)));
		case OPIN2EX:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"inexact->exact r : REAL => EXACT"));
			}
			__return(fin2ex(car(rst)));
		case OPCOERCE:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"coerce from : SEXPRESSION to : (KEYWORD | STRING | ATOM) => SEXPRESSION"));
			}
			__return(fcoerce(car(rst),car(cdr(rst))));
		case OPERROR:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"error msg : (STRING | ATOM | KEYWORD) => ERROR"));
			}
			tmp0 = car(rst);
			if(tmp0->type == STRING || tmp0->type == ATOM || tmp0->type == KEY)
			{
				__return(makeerror(3,0,tmp0->object.str));
			}
			__return(makeerror(1,0,"error's msg argument must be (STRING | ATOM | KEYWORD)"));
		case OPMKTCONC:
			itmp = pairlength(rst);
			switch(itmp)
			{
				case 0:
					__return(tconcify(e->snil));
				case 1:
					__return(tconcify(car(rst)));
				default:
					__return(makeerror(1,0,"make-tconc [s : SEXPRESSION]"));
			}
		case OPTCONC:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"tconc t : TCONC s : SEXPRESSION => tconc"));
			}
			__return(tconc(car(rst),car(cdr(rst))));
		case OPTCONCL:
			__return(e->snil);
		case OPT2P:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"tconc->pair t : TCONC => SEXPRESSION"));
			}
			if(mcar(rst)->type != TCONC)
			{
				__return(makeerror(1,0,"tconc->pair's t variable *must* be bound to a TCONC"));
			}
			__return(mcar(mcar(rst)));
		case OPTCONCSPLICE:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"tconc-splice! t : TCONC s : SEXPRESSION => TCONC"));
			}
			if(mcar(rst)->type != TCONC)
			{
				__return(makeerror(1,0,"tconc-splice!'s t arugment *must* be bound to a TCONC"));
			}
			__return(tconc_splice(car(rst),car(cdr(rst))));
		case OPMETA:
			__return(fmeta(rst));
		case OPCURTICK:
			__return(makeinteger(env->tick));
		case __PROC:
			proc = (SExp *(*)(SExp *, Symbol *))fst->object.procedure;
			if(e->snil == nil)
				printf("SNIL == NIL in __seval before procedure call!\n");
			__return(proc(rst,env));
        case __CONT:
#ifdef DEBUG
            printf("src: ");
            princ(src);
            printf("\nfst: ");
            princ(fst);
            printf("\nrst: ");
            princ(rst);
            printf("\nstate: %d\n",state);
            printf("ret: ");
            princ(ret);
            printf("\n");
            printf("__val: ");
            princ(__val);
            printf("\n");
            printf("__r: ");
            //princ(__r);
            printf("\n");
#endif
            LINE_DEBUG;
            tmp0 = rst;
            tmp1 = fst;
            stk = fst->object.closure.data;
            tmp2 = car(stk);
            stk = cdr(stk);
            LINE_DEBUG;
            src = tmp2->object.vec[0];
            fst = tmp2->object.vec[1];
            rst = tmp2->object.vec[2];
            ret = list_copy(mcar(tmp2->object.vec[3]), 1, pairlength(rst) + 1);
            tconc(ret,car(tmp0));
            LINE_DEBUG;
            env->data = (Window *)tmp2->object.vec[4];
            LINE_DEBUG;
            state = AINT(tmp2->object.vec[5]);
            LINE_DEBUG;
#ifdef DEBUG
            printf("after stack reification:\n");
            printf("src: ");
            princ(src);
            printf("\nfst: ");
            princ(fst);
            printf("\nrst: ");
            princ(rst);
            printf("\nstate: %d\n",state);
            printf("ret: ");
            princ(ret);
            printf("\n");
#endif
            goto __base;
		case OPCLONENV:
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"clone-environment e : ENVIRONMENT => ENVIRONMENT"));
			}
			if(mcar(rst)->type != ENVIRONMENT)
			{
				__return(makeerror(1,0,"clone-environment's first argument *must* be an environment!"));
			}
			__return(cloneenv(car(rst)));
		case OPDEFENV:
			if(pairlength(rst) != 3)
			{
				__return(makeerror(1,0,"bind-environment sym : SYMBOL s : SEXPRESSION e : ENVIROMENT => #v"));
			}
			tmp0 = car(cdr(cdr(rst)));
			if(tmp0->type == ATOM)
			{
				tmp0 = symlookup(tmp0->object.str,env);
				if(tmp0 == nil)
				{
					__return(makeerror(1,0,"no such symbol bound in bind-environment"));
				}
			}
			else if(tmp0->type == PAIR)
				tmp0 = __seval(tmp0,env);

			if(tmp0->type != ENVIRONMENT)
			{
				__return(makeerror(1,0,"bind-environment's last argument *must* be an ENVIROMENT"));
			}
			__return(fdef(car(rst),car(cdr(rst)),(Symbol *)tmp0->object.foreign));
		case OPSETENV:
			if(pairlength(rst) != 3)
			{
				__return(makeerror(1,0,"set!-enviroment expects exactly three arguments..."));
			}
			tmp0 = car(cdr(cdr(rst)));
			if(tmp0->type == ATOM)
			{
				tmp0 = symlookup(tmp0->object.str,env);
				if(tmp0 == nil)
				{
					__return(makeerror(1,0,"no such symbol bound in set!-environment"));
				}
			}
			else if(tmp0->type == PAIR)
				tmp0 = __seval(tmp0,env);
			
			if(tmp0->type != ENVIRONMENT)
			{
				__return(makeerror(1,0,"set!-environment's last argument *must* be an ENVIRONMENT"));
			}
			__return(fset(car(rst),car(cdr(rst)),(Symbol *)tmp0->object.foreign));
		case OPDEFAULTENV:
			__return(makeenv(e));
		case OPNULLENV:
            __return(makeenv(nil));
		case OPSTDENV: // i.e. copy the environment stack that the current lexical above is using
            __return(makeenv(env));
		case OPFROMENV:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"from-environment expects exactly two arguments..."));
			}
			tmp0 = car(rst);
			if(tmp0->type != ATOM && tmp0->type != STRING && tmp0->type != KEY)
			{
				__return(makeerror(1,0,"from-environment's first argument must be of type (ATOM|STRING|KEY)"));
			}
			tmp1 = car(cdr(rst));
			if(tmp1->type != ENVIRONMENT)
			{
				__return(makeerror(1,0,"from-environment's second argument *must* be of type ENVIRONMENT"));
			}
			tmp0 = symlookup(tmp0->object.str,(Symbol *)tmp1->object.foreign);
			if(tmp0 == nil)
			{
				__return(makeerror(1,0,"no so symbol in requested environment"));
			}
			__return(tmp0);	
        case OPWITHEXCEPT: /* with-exception-handler form */
            /* should be easy:
               - push a new lambda into the environments exception handler list
               - evaluate thunk
               */
            if(pairlength(rst) != 2)
            {
                __return(makeerror(1,0,"with-exception-handler expects two arguments, both are lambdas"));
            }
            tmp0 = car(rst);
            if(tmp0->type != CLOSURE && tmp0->type != PROCEDURE && tmp0->type != PRIM)
            {
                __return(makeerror(1,0,"with-exception-handlers first argument must be an applicable (lambda, procedure, primitive)"));
            }
            tmp1 = car(cdr(rst));
            if(tmp1->type != CLOSURE && tmp1->type != PROCEDURE && tmp1->type != PRIM)
            {
                __return(makeerror(1,0,"with-exception-handlers first argument must be an applicable (lambda, procedure, primitive)"));
            }     
            env->guards = cons(tmp0,env->guards);
            src = cons(tmp1,env->snil);
            state = __PRE_APPLY;
            goto __base;
		case __INTERNAL_RETURN:
			/* basically, this is the old __retstate;
			 * Flattening everything should help with total speed...
			 */
			__r = car(stk);
			stk = cdr(stk);
			if(__val->type == ERROR)
			{
                if(env->guards == env->snil)
                {
#ifndef NO_STACK_TRACE
                    printf("Original error expression:\n-----\n");
                    princ(src);
				    printf("\nbegin stack trace\n-----\n");
				    tmp0 = cons(__r,stk);
				    while(tmp0 != e->snil)
				    {
					    tmp1 = car(tmp0);
					    if(tmp1 == e->snil)
						    break;
					    tmp0 = cdr(tmp0);
					    printf("source: ");
					    llprinc(tmp1->object.vec[0], stdout, 1);
					    printf("\n");
				    }
#endif
				    return __val;
                }
                else
                {
                    tmp0 = car(env->guards);
                    env->guards = cdr(env->guards);
                    tmp1 = makestring(__val->object.error.message);
                    src = cons(tmp0,cons(tmp1,env->snil));
                    state = __PRE_APPLY;
                    __val = env->snil;
                    goto __base;
                }
			}
			/* internal Rationals should *always* be in lowest terms */
			if(__val->type == NUMBER && NTYPE(__val) == RATIONAL)
			{
				//printf("Made it to gcd reducer!\n");
				if((itmp = _igcd(NUM(__val),DEN(__val))) != 1)
                                {
                                        while(itmp != 1)
                                        {
                                                NUM(__val) /= itmp;
                                                DEN(__val) /= itmp;
                                                itmp = _igcd(NUM(__val),DEN(__val));
                                        }
                                }
                                if(__val->object.n->nobject.rational.den == 1)
                                {
                                        itmp = __val->object.n->nobject.rational.num;
                                        __val->object.n->type = INTEGER;
                               		__val->object.n->nobject.z = itmp;
                                }   
			}
			if(stk == e->snil && __r == e->snil)
			{
				if(__val->type == NUMBER && NTYPE(__val) == RATIONAL)
				{
					/* make sure that, when we're returning a rational, that
					 * it is already in lowest terms; this should most likely
					 * be a part of every math function...
					 */
					return __val;
				}
				return __val;	
			}
			src = __r->object.vec[0];
			fst = __r->object.vec[1];
			rst = __r->object.vec[2];
			ret = __r->object.vec[3];
			env->data = (Window *)__r->object.vec[4];
			state = AINT(__r->object.vec[5]);
			tconc(ret,__val);
			goto __base;
		default:
			break;
	}
	return e->svoid;
}

SExp *
macro_expand(SExp *s, Symbol *e)
{
	SExp *tmp0 = e->snil, *tmp1 = e->snil, *tmp2 = e->snil, *rst = e->snil, *fst = e->snil, *src = s;
	Symbol *tenv = nil, *env = e;
	/* bind variables, iterate over the list of binds, 
	 * macro-expand everything else in between
	 */
	/*printf("Macroexpand recieved: ");
	princ(src);
	printf("\n");*/
	if(mcar(src)->type != PAIR && mcar(src)->type != ATOM)
		return makeerror(1,0,"macro_expand recieved incorrect arguments...");
	if(mcar(src)->type == PAIR)
		src = car(src);
	//LINE_DEBUG;
	fst = symlookup(mcar(src)->object.str,env);
	//LINE_DEBUG;
	rst = cdr(src);
	//LINE_DEBUG;
	tmp0 = fst->object.closure.params;
	//LINE_DEBUG;
	tenv = fst->object.closure.env;
	//LINE_DEBUG;
	/*if(tmp0 == e->snil)
		printf("[!] tmp0 == e->snil\n");*/
	if(tmp0 == e->snil && rst != e->snil)
		return makeerror(1,0,"function has 0 arguments, so any amount of given arguments are incorrect...");
	new_window(env);
	//LINE_DEBUG;
	if(tmp0 != e->snil)
	{
		if(rst == e->snil)
		{
			/* before throwing an error, I simply need to check if the only 
			   parameter is a rest arg, which would mean completely
			   optional
			   */
			if(mcar(tmp0)->type != KEY)
				return makeerror(1,0,"no arguments given to macro with at least one non-optional parameter");
			tmp1 = mcar(mcdr(tmp0));
			tenv = add_env(env,tmp1->object.str,e->snil);
		}
		else
		{
			/* process args... */
			//LINE_DEBUG;
			while(tmp0 != e->snil)
			{
				/* look where we are by position, assign that variable... 
				 * need to implement :opt/:optional, which only copies a single
				 * item from tmp0, not the rest of the list...
				 */
				if(tmp0 == nil)
					printf("tmp0 == nil...(!!)\n");
				//printf("tmp0 == %s\n",typenames[tmp0->type]);
				if(mcar(tmp0)->type == KEY)
				{
					//LINE_DEBUG;
					if(!strncasecmp(mcar(tmp0)->object.str,"rest",4))
					{
						if(cdr(tmp0) == e->snil)
							return makeerror(1,0,":rest modfier found without variable name");
						tmp1 = rst;
						rst = e->snil;
					}
					else if(!strncasecmp(mcar(tmp0)->object.str,"body",4))
					{
						/* body is like rest, but checks if rst if nil first */
						if(rst == e->snil)
							return makeerror(1,0,"empty list for variable marked as :body");
						if(cdr(tmp0) == e->snil)
							return makeerror(1,0,":body modfier found without variable name");
						tmp1 = rst;
						rst = e->snil;
					}
					else if(!strncasecmp(mcar(tmp0)->object.str,"opt",3) || !strncasecmp(mcar(tmp0)->object.str,"optional",8))
					{
						if(cdr(tmp0) == e->snil)
							return makeerror(1,0,":opt modifier found without variable name");
						tmp1 = car(rst);
					}
					else
						return makeerror(1,0,"incorrect key in macro formals!");
					//LINE_DEBUG;
					tmp2 = car(cdr(tmp0));
					//LINE_DEBUG;
					tmp0 = cdr(tmp0);
					//LINE_DEBUG;
					/*if(tmp2 != nil)
					{
						printf("tmp2->type == %s\n",typenames[tmp2->type]);
						princ(tmp2);
						printf("\n");
					}*/
					if(tmp2->type == STRING || tmp2->type == ATOM || tmp2->type == KEY)
						tenv = add_env(env,tmp2->object.str,tmp1);
					//LINE_DEBUG;
				}
				else
				{
					if(rst == e->snil)
						makeerror(1,0,"too few arguments to macro...");
					//LINE_DEBUG;
					tmp1 = car(rst);
					//LINE_DEBUG;
					tmp2 = mcar(tmp0);
					//LINE_DEBUG;
					tenv = add_env(env,tmp2->object.str,tmp1);
					//LINE_DEBUG;
				}
				//LINE_DEBUG;
				tmp0 = cdr(tmp0);
				//LINE_DEBUG;
				rst = cdr(rst);
			}
			if(tmp0 != e->snil)
			{
				/*printf("tmp0 == ");
				princ(tmp0);
				printf("\n");*/
				return makeerror(1,0,"unsastisfied arguments to macro");
			}
		}
	}
	//LINE_DEBUG;
	tmp0 = fst->object.closure.data;
	//LINE_DEBUG;
	/* iterate over tmp0 (data), with the constructed env... */
	while(tmp0 != e->snil)
	{
		/*printf("car(tmp0) == ");
		princ(car(tmp0));
		printf("\n");
		printf("tenv == nil? %s\n", tenv == nil ? "#t" : "#f");*/
		tmp2 = car(tmp0);
		/*printf("Attempting to eval: ");
		princ(tmp2);
		printf("\n");*/
		if(tmp2->type == PAIR)
			tmp2 = lleval(car(tmp0),tenv);
		else if(tmp2->type == ATOM)
			tmp2 = symlookup(mcar(tmp0)->object.str,tenv);
		else if(tmp2->type == ERROR)
			return tmp2; /* automatically return on errors... */
		tmp0 = cdr(tmp0);
	}
	/*printf("Macroexpand returned: ");
	princ(tmp2);
	printf("\n");*/
	close_window(env);
	return tmp2;
}

/* __build: iterate over a pair, replacing atoms with anything from the alist,
 * recursing over pairs, and consing anything else in place
 */
SExp *
__build(SExp *src, SExp *alist, Symbol *e)
{
    return e->snil;
}

SExp *
syntax_expand(SExp *s, Symbol *e)
{
    SExp *piter = e->snil, *siter = e->snil, *pattern = e->snil;
    SExp *sobj = e->snil, *tmp = e->snil, *src = e->snil, *tmp1 = e->snil;
    Trie *keylist = nil;

    printf("src == ");
    princ(src);
    printf("\n");
    tmp = car(src);
    sobj = symlookup(tmp->object.str,e);
    if(sobj == nil)
        return makeerror(1,0,"syntax-expand: no such syntax object found");
    printf("sobj == ");
    princ(sobj->object.closure.data);
    printf("\n");
    tmp = car(sobj);
    sobj = cdr(sobj);
    if(tmp != e->snil)
    {
        /* so, if we have a pair, iterate over each memeber, checking if it
         * is an atom, and add it to a dictionary. This dictionary serves as 
         * a cheaper mechanism for lookups (at least, cheaper than linear list
         * scans. keylist is *not* allocated unless there is a pair passed in
         * here, so I still have to check below if keylist is nil or not before
         * doing a keyword check (which isn't a huge deal)
         */
        if(tmp->type != PAIR)
            return makeerror(1,0,"syntax-expand: keywords must be organized in a PAIR");
        keylist = (Trie *)hmalloc(sizeof(Trie));
        keylist->n_len = 1;
        keylist->n_cur = 1;
        keylist->nodes = (Trie **)hmalloc(sizeof(Trie *));
        keylist->data = nil;
        while(tmp != e->snil)
        {
            piter = car(tmp);
            if(piter->type != ATOM)
                return makeerror(1,0,"syntax-expand: keywords must be ATOMs");
            trie_put(piter->object.str,e->strue,keylist); 
            tmp = cdr(tmp);
        }
    }
    while(sobj != e->snil)
    {
        /* attempt to match a pattern */
        pattern = car(car(sobj));
        tmp = car(cdr(car(sobj)));
        src = s; 
        while(pattern != e->snil)
        {
            piter = car(pattern);   
            siter = car(src);
            pattern = cdr(pattern);
            src = cdr(src);
            if(piter->type == ATOM)
            {
                tmp1 = trie_get(piter->object.str,keylist);
                if(tmp1 == nil) /* not a keyword */
                {
                    if(mcdr(pattern) != e->snil)
                    {
                        tmp1 = car(cdr(pattern));
                        if(tmp1->type == ATOM && !strcmp("...",tmp1->object.str))
                        {
                            /* we have found a ellipsis pattern; if cdr(cdr(pattern))
                             * == snil, associate the rest of the src with this pattern
                             * object, otherwise calculate how many objects from src should
                             * be associated (i.e. (_ foo ... f e) means slice from 0 to
                             * len(src) - 2, for f & e
                             */
                        }
                        else
                        {
                            /* just normal association */
                        }
                    }
                    else
                    {
                    }
                }
                else
                {
                    tmp1 = eqp(siter,piter); /* keywords must match themselves */
                    if(!tmp1->object.c)
                        break;
                }
            }
            else if(piter->type == PAIR)
            {
                return e->snil;
            }
            else if(piter->type == VECTOR)
            {
                return e->snil;
            }
            else
            {
                return e->snil;
            }
        }
        sobj = cdr(sobj);
    }
    return makeerror(1,0,"syntax-expand: no matching patterns found");
}
/* syntactic functions */
SExp *
fset(SExp *tmp0, SExp *tmp1, Symbol *env)
{
	Symbol *tenv = env;
	int itmp = 0;
	Window *hd = nil;
	Trie *data = nil;
	SExp *d = env->snil;
	if(tmp0->type != ATOM)
		return makeerror(1,0,"set! first argument *must* be an atom");
	if(tmp1->type == PAIR)
		tmp1 = lleval(tmp1,env);
	else if(tmp1->type == ATOM)
		tmp1 = symlookup(tmp1->object.str,env);
	if(tmp1->type == ERROR)
		return tmp1;
	// need to check if the atom exists first, since this
	// will define as well...
	if(env == nil || env->data == nil)
		return env->snil;
	hd = env->data;
	while(hd != nil)
	{
		data = hd->env;
		d = trie_get(tmp0->object.str,data);
		if(d != nil)
		{
			trie_put(tmp0->object.str,tmp1,data);
			return env->svoid;
		}
		hd = hd->next;
	}
	return makeerror(1,0,"set!: unknown symbol used in set form");
}
SExp *
fdef(SExp *tmp0, SExp *tmp1, Symbol *env)
{
    SExp *d = nil, *tmp2 = nil;
    if(tmp0->type == PAIR)
    {
        d = car(tmp0);
        if(d->type != ATOM)
            return makeerror(1,0,"the first member of a procedure-define *must* be an ATOM");
        /*princ(tmp0);
        printf("\n");
        princ(tmp1);
        printf("\n");*/
        tmp2 = ffn(cons(cdr(tmp0),cdr(tmp1)),env);
        if(tmp2->type == ERROR)
            return tmp2;
        add_env(env,d->object.str,tmp2);
        return env->svoid; 
    }
    else if(tmp0->type == ATOM)
    {
        tmp1 = car(cdr(tmp1));
        if(tmp1->type == PAIR)
            tmp1 = lleval(tmp1,env);
        else if(tmp1->type == ATOM)
        {
            tmp1 = symlookup(tmp1->object.str,env);
            if(tmp1 == nil)
                return makeerror(1,0,"unknown atom in def's second argument!");
        }
        if(tmp1->type == ERROR)
            return tmp1;
        add_env(env,tmp0->object.str,tmp1);
        return env->svoid;
    }
    return makeerror(1,0,"def's first argument *must* be SYMBOL | PAIR");
}
SExp *
fdefmacro(SExp *tmp0, SExp *tmp1, SExp *body, Symbol *env)
{
	SExp *tmp2 = nil;
	if(tmp0->type != ATOM)
		return makeerror(1,0,"first argument to define-macro must be a symbol");
	if(tmp1->type != PAIR)
		return makeerror(1,0,"second argument to define-macro must be a pair");
	tmp2 = (SExp *) hmalloc(sizeof(SExp));
	tmp2->type = MACRO;
	tmp2->object.closure.params = tmp1;
	tmp2->object.closure.data = body;
	env = add_env(env,tmp0->object.str,tmp2);
	return env->svoid;
}
SExp *
fdefsyntax(SExp *tmp0, SExp *body, Symbol *env)
{
	SExp *tmp1 = nil;
	if(tmp0->type != ATOM)
		return makeerror(1,0,"define-syntax syntax name *must* be a symbol");
	tmp1 = (SExp *)hmalloc(sizeof(SExp));
	tmp1->type = SYNTAX;
	tmp1->object.closure.data = body; /* should probably check what's going on here... */
	env = add_env(env,tmp0->object.str,tmp1);
	return env->svoid;
}
SExp *
ffn(SExp *rst, Symbol *env)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil, *tmp3 = nil;
	/* need to add checks for positional args after 
	   keyword args...
	   Basically, we're making SRFI-89 (which is, by the way, completely
	   insane. Witness:
	   	(define* (h2 (key: k #f) a . r) (list a k r))
		(define* (h1 a (key: k #f) . r) (list a k r))
	   Normal. Nothing out of the ordinary. Until you attempt to call them...
	   	(h2 key: 2 1 3 4 5) ; => (1 2 (3 4 5))
		(h2 1 key: 2 3 4 5) ; => (1 #f (key: 2 3 4 5))
		(h1 1 key: 2 3 4 5) ; => (1 2 (3 4 5))
		(h1 key: 2 1 3 4 5) ; => (key: #f (2 1 3 4 5))
	   This may be STklos' implementation of SRFI-89, but it seems to be the
	   standard one. What is the point of keyword arguments if they cannot
	   appear in any order *after* positional arguments? That's why I changed
	   the grammar of SRFI-89 (see the closure eval section below; also in the
	   docs)) act like Python: 
	   	- No keyword arguments before positional arguments
		- No multiple assigns to arguments
		- Ability to specify keyword arguments by position
	   Also, types, default values & constraints may be specified for individual
	   arguments; pre/post conditions/contracts should be supported as well.
	   On top of these things, help & specification strings should be supported:
	   	(define myfn (fn (x) "This is a help string" "this is a specification String" x))
	   	(help 'myfn) ; => "This is a help string"
	   	(spec 'myfn) ; => "this is a specification string" 
	   	(fn ((x: x type: :integer) (y: y type: :integer default: 3) -> :pre (.>=. x y) :post (.>=. (+ x y) x y)) (+ x y))
	   While it is an error to have a keyword'd object before a positional argument, it is not an error to specify a
	   type, a constraint or a default value for a positional argument:
	   	(fn ((x type: :integer) a) ...) ; => typed positional parameter
		(fn ((x default: 3) a) ...) ; => this isn't an error, but probably should be (a should either have a default or be opt:)
	   The default issue should be handled like Python: as an error for functions...
	 */
	/*printf("Rst: ");
	princ(rst);
	printf("\n");*/
	if(pairlength(rst) < 2)
		return makeerror(1,0,"fn (bindings*) (form*) => closure");
	tmp0 = (SExp *)hmalloc(sizeof(SExp));
	tmp0->type = CLOSURE;
	tmp1 = car(rst);
	if(tmp1->type != PAIR && tmp1->type != NIL && tmp1->type != ATOM)
		return makeerror(1,0,"bindings must be either a PAIR, NIL or a SYMBOL");
	if(tmp1->type == PAIR)
	{
		/* let's check args, to test for what was mentioned above... */
		while(tmp1 != env->snil)
		{
			tmp2 = car(tmp1);
			if(tmp2->type != PAIR && tmp2->type != ATOM && tmp2->type != KEY)
				return makeerror(1,0,"formals must be either a pair or an atom");
			if(tmp2->type == PAIR)
			{
				if(mcar(tmp2)->type == KEY && mcar(mcdr(tmp1))->type == ATOM)
					return makeerror(1,0,"a positional argument may not come after a keyword argument");
                tmp3 = car(cdr(tmp2));
                switch(tmp3->type)
                {
                    case PAIR:
                        tmp3 = __seval(tmp3,env);
                        if(tmp3 == nil || tmp3->type == ERROR)
                            return makeerror(1,0,"evaluation of optional argument default failed");
                        mcar(mcdr(tmp2)) = tmp3;
                        break;
                    case ATOM:
                        tmp3 = symlookup(tmp3->object.str,env);
                        if(tmp3 == nil)
                            return makeerror(1,0,"optional argument is unknown at instantiation time");
                        mcar(mcdr(tmp2)) = tmp3;
                        break;
                    default:
                        break;
                }
			}
			else if(tmp2->type == KEY)
				if(strcasecmp("rest",tmp2->object.str) && strcasecmp("body",tmp2->object.str) && strcasecmp("opt",tmp2->object.str))
					return makeerror(1,0,"the only keyword objects for function definition are opt, rest & body");
			tmp1 = cdr(tmp1);
            if(tmp1->type == ATOM) // hit a rest arg
                break;
		}
	}
	//tmp0->object.closure.env = (void *)shallow_clone_env(env);
	tmp0->object.closure.env = (void *)env->data;
	tmp0->object.closure.params = car(rst);
	tmp2 = car(cdr(rst));
	if(tmp2->type == STRING)
	{
		//tmp0->object.closure.docstr = tmp2;
		tmp0->metadata = (Trie *)hmalloc(sizeof(Trie));
		tmp0->metadata->key = nul;
		tmp0->metadata->n_len = 0;
		tmp0->metadata->n_cur = 0;
		tmp0->metadata->nodes = nil;
		trie_put("docstring",tmp2,tmp0->metadata);
		tmp0->object.closure.data = cdr(cdr(rst));
	}
	else
		tmp0->object.closure.data = cdr(rst);
	return tmp0;
}

SExp *
f_load(SExp *s, Symbol *env)
{
	FILE *fdin = nil;
	SExp *ret = env->snil;
	int exprcnt = 0;
	/*printf("Rigors (in f_load): \n");
	if(ret != nil)
	{
		printf("ret->type == %s\n",typenames[ret->type]);
		printf("ret == env->snil? %d\n",(ret == env->snil ? 1 : 0));
	}
	else
		printf("Ret == nil(!!)\n");
	*/
	if(pairlength(s) != 1)
		return makeerror(1,0,"load expects only one argument, a string for a path name...");
	if((fdin = fopen(mcar(s)->object.str,"r")) == nil)
		return makeerror(1,0,"load: cannot open file...");
	while(1)
	{
		ret = llread(fdin);
		exprcnt++;
		//llprinc(ret,stderr,1);
		if(ret->type == ERROR)
			break;
		else if(ret == env->seof)
			break;
		ret = lleval(ret,env);
		if(quit_note)
			break;
		if(ret->type == ERROR)
			break;
	}
	if(ret->type == ERROR)
	{
		printf("Error on expression: %d\n",exprcnt);
		return ret;
	}
	return env->svoid;	
}
