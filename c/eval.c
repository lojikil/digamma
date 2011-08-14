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

SExp *
__seval(SExp *s, Symbol *e)
{
    SExp *src = s, *fst = snil, *rst = snil, *tmp0 = snil, *tmp1 = snil;
    SExp *tmp2 = snil, *stk = snil, *__r = snil, *__val = snil, *ret = snil;
	Symbol *tenv = nil, *env = e;
	int state = __PRE_APPLY, itmp = 0,tail = 0; /* tail is a flag for __PRE_APPLY & OPBEGIN */
	SExp *(*proc)(SExp *, Symbol *) = nil;
	if(s == nil)
		return svoid;
	env = shallow_clone_env(e);
__base:
	/*printf("Stack depth: %d; State: %d\n",pairlength(stk),state);
	printf("src == ");
	princ(src);
	printf("\n");*/
	// add interpreter tick here.
	switch(state)
	{
		case __PRE_APPLY:
			e->tick++;
			if(src->type == PAIR)
			{
				/*printf("ret(0) == ");
				princ(ret);
				printf("\n");
				if(ret != snil && mcar(ret) != snil)
					fst = mcar(mcar(ret));
				else*/
				fst = car(src);
				/*printf("fst == ");
				princ(fst);
				printf("\n");*/
				rst = cdr(src);
				ret = tconcify(snil);
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
				if(fst->type != PRIM && fst->type != PROCEDURE && fst->type != VECTOR && fst->type != STRING && fst->type != DICT && fst->type != CLOSURE)
				{
					__return(makeerror(1,0,"invalid type for application: only primitives, procedures, vectors, strings & dictionaries may be applied"));
				}
			}
			else
			{
				__return(src);
			}
			//goto __base;
		case __POST_APPLY:
			tmp0 = rst;
			/* it would be great if we could unify closure arguments
			 * whilst processing args for application, but for now
			 * I'll keep the two separate
			 */
			while(tmp0 != snil)
			{
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
			switch(fst->type)
			{
				case PRIM:
					state = fst->object.primitive.num;
					break;
				case PROCEDURE:
					state = __PROC;
					break;
				case CLOSURE:
					// unify formal parameters with arguments
					// Need to add a check here for a tail call condition.
					env->data = (Window *) fst->object.closure.env;
					tmp0 = fst->object.closure.params;
					/* add test for tail here... */
					if(rst == snil)
					{
						if(tmp0->type != NIL && mcar(tmp0)->type != KEY)
						{
							// attempt a stack trace
							printf("Stack trace: \n");
							tmp1 = stk;
							while(tmp1 != snil)
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
					while(tmp0 != snil)
					{
						tmp1 = car(tmp0);
						if(tmp1->type == ATOM)
						{
							if(rst == snil)
							{
								__return(makeerror(1,0,"Unsatisfied non-optional procedure argument"));
							}
							add_env(env,tmp1->object.str,car(rst));
						}
                                                else if(tmp1->type == PAIR)
						{
                                                        tmp2 = tmp1;
							tmp1 = car(tmp2);
							tmp2 = car(cdr(tmp2));
							if(rst != snil)
                                                        {
							    add_env(env,tmp1->object.str,car(rst));
                                                            rst = cdr(rst);
                                                        }
							else
							{
							    tmp2 = __seval(tmp2,env);
						            add_env(env,tmp1->object.str,tmp2);
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
				__return(makeerror(1,0,"cdr list : PAIR => S-EXPRESSION"));
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
			src = cons(car(rst),car(cdr(rst)));
			state = __PRE_APPLY;
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
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"nth (c : COLLECTION) (idx: INTEGER) => s-expression"));
			}
			__return(fnth(car(rst),car(cdr(rst))));
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
			__return(mcar(trie_keys(tmp0->object.dict,tconcify(snil))));
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
			__return(trie_partial(tmp0->object.dict,tmp1->object.str,0));
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
			__return(snil);
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
			//printf("Made it to OPIF\n");
			if(itmp < 2 && mcar(ret) == snil)
			{
				__return(makeerror(1,0,"if c : S-EXPRESSION if-true : S-EXRESSION [if-false : S-EXPRESSION] => S-EXPRESSION"));
			}
			if((ret->type == TCONC && mcar(ret) == snil) || ret->type != TCONC) /* should be the initial eval... */
			{
				tmp0 = car(rst);
				if(tmp0->type == PAIR)
				{
					// jump schuck
					stk = cons(vector(6,src,fst,cdr(rst),ret,env->data,makeinteger(state)),stk);
					src = tmp0;
					state = __PRE_APPLY;
					goto __base;
				}
				else if(tmp0->type == ATOM)
				{
					tmp0 = symlookup(tmp0->object.str,env);
					if(tmp0 == nil)
					{
						 __return(makeerror(1,0,"unknown symbol in if condtion"));
					}
				}
				// check for truth or falsity, eval based upon this...
				if(tmp0->type == BOOL || tmp0->type == GOAL)
				{
					if(!tmp0->object.c)
					{
						tmp1 = car(cdr(cdr(rst)));
						if(tmp1->type == PAIR)
						{
							src = tmp1;
							state = __PRE_APPLY;
							goto __base;
						}
						else if(tmp1->type == ATOM)
						{
							tmp1 = symlookup(tmp1->object.str,env);
							if(tmp1 == nil)
							{
								__return(makeerror(1,0,"unknown symbol in if then clause"));
							}
						}
						if(tmp1 == snil && cdr(cdr(rst)) == snil)
						{
							__return(sfalse);
						}
						__return(tmp1);
					}
					else
					{
						tmp1 = car(cdr(rst));
						if(tmp1->type == PAIR)
						{
							src = tmp1;
							state = __PRE_APPLY;
							goto __base;
						}
						else if(tmp1->type == ATOM)
						{
							tmp1 = symlookup(tmp1->object.str,env);
							if(tmp1 == nil)
							{
								__return(makeerror(1,0,"unknown symbol in if then clause"));
							}
						}
						if(tmp1 == snil && cdr(rst) == snil)
						{
							__return(sfalse);
						}
						__return(tmp1);
					}
				}
			}
			else
			{
				/* ok, so there must have been a pair for the if-conditional element, so 
				 * we have to look at it's result, and jump from there.
				 * this section of if should be much cleaner than the above (which can, I *think*
				 * be cleaned up quite a bit).
				 */
				 if(ret->type != TCONC)
				 {
				 	__return(makeerror(1,0,"internal error"));
				 }
				 tmp0 = mcar(mcar(ret));
				if(tmp0->type == BOOL || tmp0->type == GOAL)
				 {
					if(!tmp0->object.c)
					{
						tmp1 = car(cdr(rst));
						if(tmp1->type == PAIR)
						{
							src = tmp1;
							state = __PRE_APPLY;
							goto __base;
						}
						else if(tmp1->type == ATOM)
						{
							tmp1 = symlookup(tmp1->object.str,env);
							if(tmp1 == nil)
							{
								__return(makeerror(1,0,"unknown symbol in if then clause"));
							}
						}
						if(tmp1 == snil && cdr(rst) == snil)
						{
							__return(sfalse);
						}
						__return(tmp1);
					}
					else
					{
						tmp1 = car(rst);
						if(tmp1->type == PAIR)
						{
							src = tmp1;
							state = __PRE_APPLY;
							goto __base;
						}
						else if(tmp1->type == ATOM)
						{
							tmp1 = symlookup(tmp1->object.str,env);
							if(tmp1 == nil)
							{
								__return(makeerror(1,0,"unknown symbol in if then clause"));
							}
						}
						if(tmp1 == snil && cdr(rst) == snil)
						{
							__return(sfalse);
						}
						__return(tmp1);
					}
				 }
			}
			__return(sfalse);
		case OPUNWIND:
			__return(snil);
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
					if(fst->type == CLOSURE && tmp0 == snil)
					{
						/* Ok, so what we're doing here:
						 * if it's a tail call in a closure,
						 * and we're calling the same procedure,
						 * set tail = 1, so that we modify parameters in place
						 * otherwise, just use a normal window
						 */
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
					tconc(ret,tmp1);
				}
				else
					tconc(ret,tmp1);
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
			if(tmp1->type != PAIR)
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
			if(tmp1->type != PAIR)
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
					__return(tconcify(snil));
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
			__return(snil);
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
			if(env->snil == nil)
				printf("SNIL == NIL in __seval before procedure call!\n");
			__return(proc(rst,env));
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
		case __INTERNAL_RETURN:
			/* basically, this is the old __retstate;
			 * Flattening everything should help with total speed...
			 */
			__r = car(stk);
			stk = cdr(stk);
			if(__val->type == ERROR)
			{
#ifndef NO_STACK_TRACE
				printf("\nbegin stack trace\n-----\n");
				tmp0 = cons(__r,stk);
				while(tmp0 != snil)
				{
					tmp1 = car(tmp0);
					if(tmp1 == snil)
						break;
					tmp0 = cdr(tmp0);
					printf("source: ");
					princ(tmp1->object.vec[0]);
					printf("\n");
				}
#endif
				return __val;
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
			if(stk == snil && __r == snil)
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
	return svoid;
}

SExp *
macro_expand(SExp *s, Symbol *e)
{
	SExp *tmp0 = snil, *tmp1 = snil, *tmp2 = snil, *rst = snil, *fst = snil, *src = s;
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
	/*if(tmp0 == snil)
		printf("[!] tmp0 == snil\n");*/
	if(tmp0 == snil && rst != snil)
		return makeerror(1,0,"function has 0 arguments, so any amount of given arguments are incorrect...");
	new_window(env);
	//LINE_DEBUG;
	if(tmp0 != snil)
	{
		if(rst == snil)
		{
			/* before throwing an error, I simply need to check if the only 
			   parameter is a rest arg, which would mean completely
			   optional
			   */
			if(mcar(tmp0)->type != KEY)
				return makeerror(1,0,"no arguments given to macro with at least one non-optional parameter");
			tmp1 = mcar(mcdr(tmp0));
			tenv = add_env(env,tmp1->object.str,snil);
		}
		else
		{
			/* process args... */
			//LINE_DEBUG;
			while(tmp0 != snil)
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
						if(cdr(tmp0) == snil)
							return makeerror(1,0,":rest modfier found without variable name");
						tmp1 = rst;
						rst = snil;
					}
					else if(!strncasecmp(mcar(tmp0)->object.str,"body",4))
					{
						/* body is like rest, but checks if rst if nil first */
						if(rst == snil)
							return makeerror(1,0,"empty list for variable marked as :body");
						if(cdr(tmp0) == snil)
							return makeerror(1,0,":body modfier found without variable name");
						tmp1 = rst;
						rst = snil;
					}
					else if(!strncasecmp(mcar(tmp0)->object.str,"opt",3) || !strncasecmp(mcar(tmp0)->object.str,"optional",8))
					{
						if(cdr(tmp0) == snil)
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
					if(rst == snil)
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
			if(tmp0 != snil)
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
	while(tmp0 != snil)
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
__build(SExp *src, SExp *alist)
{
	SExp *holder = nil, *iter = nil, *ret = nil, *tmp = nil, *name = nil;
	if(src == nil)
		return nil;
	if(src->type == ATOM)
	{
		holder = assq(src,alist);
		if(holder == nil)
			return src;
		return car(cdr(holder));
	}
	else if(src->type == PAIR)
	{
		//printf("Am I looking at the correct area?\n");
		holder = src;
		ret = cons(nil,snil);
		tmp = ret;
		while(holder != snil)
		{
			iter = car(holder);
			/*printf("iter: ");
			princ(iter);
			printf("\n");*/
			if(iter->type == PAIR)
				iter = __build(iter,alist);
			else if(iter->type == ATOM)
			{
				name = assq(iter,alist);
				/*printf("iter == ");
				princ(iter);
				printf("\n");
				printf("name == ");
				princ(name);
				printf("\n");*/
				if(name != nil)
					iter = car(cdr(name));
				/*printf("iter == ");
				princ(iter);
				printf("\n");*/
			}
			mcar(tmp) = iter;
			/*printf("tmp: \n\t");
			princ(tmp);
			printf("\n");*/
			holder = cdr(holder);
			if(holder != snil)
			{	
				mcdr(tmp) = cons(snil,snil);
				tmp = mcdr(tmp);
			}
		}
		return ret;
	}
	return src;
}

SExp *
syntax_expand(SExp *src, Symbol *e)
{
	SExp *base = nil, *rules = nil, *t0 = nil, *t1 = nil, *t2 = nil, *zip = nil;
	if(src->type != PAIR)
		return makeerror(1,0,"syntax-expand: not syntax");
	if(mcar(src)->type == PAIR)
		src = car(src);
	t0 = car(src);
	if(t0->type == ATOM)
	{
		t0 = symlookup(t0->object.str,(Symbol *)e);
		if(t0->type != SYNTAX)
			return makeerror(1,0,"syntax-expand: not syntax");
	}
	rules = t0->object.closure.data;
	if(rules->type != PAIR)
		return makeerror(1,0,"syntax-expand: mal-formed rules");
	while(rules != snil)
	{
	
		rules = cdr(rules);	
	}
	return snil;
}
