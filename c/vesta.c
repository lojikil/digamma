/* @(#) Reference implementation for Digamma 2009.3
 *
()
  ()
()  ()
Digamma/Vesta 2009.3
 * I think this one fits nicely with the whole simplicity theme ^_^
 * Also, it means Phyla's "brand" will have a distinctive logo, one
 * not necessarily tied to F.
 */
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
const char *typenames[] = {
        "Symbol", "Number","Character","Boolean","Goal", "Vector",
	"Pair","String","Procedure","Closure","Foreign", "Nil",
	"Error","Port","Macro","User","TConc","Primitive",
	"Dictionary","Key","Syntax","End of File","Void",
	"Environment","Unsafe Foreign","Continuation",0
};
const char *toknames[] = {
        "TOK_LPAREN",
        "TOK_RPAREN",
        "TOK_LSQUAR",
        "TOK_RSQUAR",
        "TOK_LCURLY",
        "TOK_RCURLY",
        "TOK_INT",
        "TOK_LITSTR",
        "TOK_NQUOTE",
        "TOK_MQUOTE",
        "TOK_UNQUOT",
        "TOK_SPLICE",
        "TOK_SYMBOL",
        "TOK_CHAR",
        "TOK_TRUE",
        "TOK_FALSE",
        "TOK_SUCC",
        "TOK_UNSUCC",
        "TOK_NAME",
        "TOK_REAL",
        "TOK_RATIO",
        "TOK_COMPL",
        "TOK_KEY",
        "TOK_HERROR",
        "TOK_EOF",
        0
};
extern char **environ;
extern int h_errno;
extern int errno;
const char *numtypes[] = {"Integer", "Real", "Rational", "Complex",0};
static SExp *snil = nil, *sfalse = nil, *strue = nil, *ssucc = nil, *sunsucc = nil,*mem_err = nil, *pinf = nil, *ninf = nil, *qnan = nil, *snan = nil;
static SExp *fake_rpar = nil, *fake_rsqr = nil, *fake_rcur = nil; /* returns for llread, but should never really mean anything */
static SExp *seof = nil, *svoid = nil; /* #e, for use with read*, & eof-object?, #v for void */
static int cons_cnt = 0, gensymglobal = 0;
int quit_note = 0;


SExp *
car(SExp *s)
{
	if(s->type == PAIR)
		return s->object.clist.first;
	return s;
}
SExp *
cdr(SExp *s)
{
	if(s->type == PAIR)
		return s->object.clist.rest;
	return s;
}
SExp *
cons(SExp *s0, SExp *s1)
{
	SExp *ret = nil;
	if((ret = (SExp *)hmalloc(sizeof(SExp))) == nil)
		return mem_err;
	ret->type = PAIR;
	ret->object.clist.first = s0;
	ret->object.clist.rest = s1;
	cons_cnt++;
	return ret;
}
SExp *
snoc(SExp *s0, SExp *s1)
{
	SExp *ret = nil;
	printf("%s %s\n",typenames[TYPE(s0)],typenames[TYPE(s1)]);
	if(s0->type == PAIR)
	{
		ret = s0;
		while(ret->object.clist.rest != snil)
			ret = ret->object.clist.rest;
		ret->object.clist.rest = cons(s1,snil);
		return s0; /* yes, snoc is destructive! */
	}
	else
		return cons(s0,cons(s1,snil));
}
SExp *
tconc(SExp *s0, SExp *s1)
{
	SExp *tmp = nil;
	if(s0->type != TCONC)
		return makeerror(1,0,"tconc: type clash");
	if(mcar(s0)->type != NIL)
	{
		tmp = mcdr(s0);
		mcdr(tmp) = cons(s1,snil);
		mcdr(s0) = mcdr(tmp);
	}
	else
	{
		tmp = cons(s1,snil);
		mcar(s0) = tmp;
		mcdr(s0) = tmp;
	}
	return svoid;
}
SExp *
tconc_splice(SExp *s0, SExp *s1)
{
	SExp *tmp = nil, *holder = nil;
	if(s0->type != TCONC)
		return makeerror(1,0,"tconc: type clash");
	/*printf("\n-------------------\ntconc_splice\ns0: ");
	princ(s0);
	printf("\ns1: ");
	princ(s1);*/
	if(mcar(s0)->type != NIL)
	{
		//printf("\n\t\tPath 0");
		tmp = mcdr(s0);
		if(s1->type == PAIR)
		{
			//printf("\n\t\t\tPath 0.0");
			holder = tconcify(s1);
			//printf("\n\t\t\tholder == ");
			//princ(holder);
			mcdr(tmp) = mcar(holder);
			mcdr(s0) = mcdr(holder);
		}
		else if(s1->type == TCONC)
		{
			/*printf("\n\t\t\tPath 0.1");
			printf("\n\t\t\ts0 == ");
			princ(s0);
			printf("\n\t\t\ts1 == ");
			princ(s1);
			printf("\n\t\t\ttmp == ");
			princ(tmp);*/
			/* ok, this is fscking up, because we're not actually adding pairs
			 * to the tconcs properly; we need to fix 0.0, to make the end of the
			 * tconc point to the end of the pair that's being spliced in
			 */
			mcdr(tmp) = mcar(s1);
			mcdr(s0) = mcdr(s1);
		}
		else
		{
			//printf("\n\t\t\tPath 0.2");
			mcdr(tmp) = cons(s1,snil);
			mcdr(s0) = mcdr(tmp);
		}
	}
	else
	{
		//printf("\n\t\tPath 1");
		if(s1->type == PAIR)
		{
			tmp = tconcify(s1);
			mcar(s0) = tmp;
			mcdr(s0) = tmp;
		}
		else if(s1->type == TCONC)
		{
			mcar(s0) = mcar(s1);
			mcdr(s0) = mcdr(s1);
		}
		else
		{	
			tmp = cons(s1,snil);
			mcar(s0) = tmp;
			mcdr(s0) = tmp;
		}
	}
	//printf("\n-------------------\n");
	return svoid;
}
/* Make a tconc-structure from a PAIR s */
SExp *
tconcify(SExp *s)
{
	SExp *ret = snil, *tmp = s;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = TCONC;
	if(s->type == PAIR)
	{
		while(1)
		{
			if(mcdr(tmp) == snil)
				break;
			tmp = cdr(tmp);
		}
		ret->object.clist.first = s;
		ret->object.clist.rest = tmp;
	}
	else
	{
		if(s == snil)
		{
			mcar(ret) = snil;
			mcdr(ret) = snil;
		}
		else
		{
			tmp = cons(s,snil);
			mcar(ret) = tmp;
			mcdr(ret) = tmp;
		}
	}
	return ret;
}
SExp *
bappend(SExp *s0, SExp *s1)
{
	SExp *ret = snil, *tmp0 = snil, *tmp1 = snil, *tmp2 = snil;
	if(s1 == snil)
		return s0;
	if(s0 == snil)
		return s1;
	if(s0->type != PAIR)
		return makeerror(2,0,"bappend error");
	ret = cons(snil,snil);
	tmp0 = ret;
	tmp1 = s0;
	while(tmp1 != snil)
	{
		tmp2 = car(tmp1);
		mcar(tmp0) = tmp2;
		tmp1 = cdr(tmp1);
		if(tmp1 == snil)
			break;
		mcdr(tmp0) = cons(snil,snil);
		tmp0 = mcdr(tmp0);
	}
	mcdr(tmp0) = s1;
	return ret;
}
SExp *
append(SExp *rst)
{
	/*SExp *ret = snil, *holder = nil, *tmp = snil;
	holder = s;
	if(holder->type != PAIR)
		return makeerror(1,0,"Type clash: append plist => list");
	while(holder != snil)
	{
		tmp = car(holder);
		if(tmp->type != PAIR)
			return makeerror(1,0,"Type clash: append plist => list");
		while(tmp != snil)
		{
			ret = snoc(ret,car(tmp));
			tmp = cdr(tmp);
		}
		holder = cdr(holder);
	}
	return ret;*/
	int itmp = 0;
	SExp *tmp0 = snil, *tmp1 = snil, *tmp2 = snil;
	itmp = pairlength(rst);
	switch(itmp)
	{
		case 0:
			return snil;
		case 1:
			return car(rst);
		default:
			tmp1 = cons(snil,snil);
			tmp2 = tmp1;
			while(rst != snil)
			{
				if(mcdr(rst) == snil)
					break;
				tmp0 = car(rst);
				if(tmp0 == snil)
				{
					rst = cdr(rst);
					continue;
				}
				if(tmp0->type != PAIR)
					return makeerror(1,0,"append (l*: PAIR) (e : S-EXPRESSION) => PAIR");
				while(tmp0 != snil)
				{
					if(mcar(tmp1) != snil)
					{
						mcdr(tmp1) = cons(snil,snil);
						tmp1 = mcdr(tmp1);
					}
					mcar(tmp1) = mcar(tmp0);
					tmp0 = mcdr(tmp0);
				}
				rst = cdr(rst);
			}
			mcdr(tmp1) = mcar(rst);
			return tmp2;
	}
}
int
pairlength(SExp *s)
{
	int i = 0;
	SExp *holder = s;
	if(s == nil || s->type != PAIR)
		return 0;
	while(holder != snil)
	{
		i++;
		if(holder->type != PAIR) // improper list
			break;
		holder = cdr(holder);
	}
	return i;
}
SExp *
eqp(SExp *s0, SExp *s1)
{
	SExp *tmp0 = s0, *tmp1 = s1;
	if(tmp0->type != tmp1->type)
		return sfalse;
	switch(tmp0->type)
	{
		case NUMBER:
			if(tmp0->object.n->type != tmp1->object.n->type)
				return sfalse;
			switch(tmp0->object.n->type)
			{
				case INTEGER:
					if(tmp1->object.n->nobject.z == tmp0->object.n->nobject.z)
						return strue;
					return sfalse;
				case REAL:
					if(tmp1->object.n->nobject.real == tmp0->object.n->nobject.real)
						return strue;
					return sfalse;
				case RATIONAL:
					if(tmp1->object.n->nobject.rational.num != tmp0->object.n->nobject.rational.num)
						return sfalse;
					if(tmp1->object.n->nobject.rational.den != tmp0->object.n->nobject.rational.den)
						return sfalse;
					return strue;
				case COMPLEX:
					if(tmp1->object.n->nobject.complex.r != tmp0->object.n->nobject.complex.r)
						return sfalse;
					if(tmp1->object.n->nobject.complex.i != tmp0->object.n->nobject.complex.i)
						return sfalse;
					return strue;
			}
			break;
		case KEY:
		case STRING:
		case ATOM:
			if(!strcasecmp(tmp0->object.str,tmp1->object.str))
				return strue;
			return sfalse;
		case CHAR:
			if(tmp0->object.c == tmp1->object.c)
				return strue;
			return sfalse;
		case NIL: /* since we know both type(tmp0) and type(tmp1) == NIL, there is no other test required */
			return strue; 
		default:
			if(tmp0 == tmp1)
				return strue;
			return sfalse;
	}
	return sfalse;
}
SExp *
assq(SExp *item, SExp *alist)
{
	SExp *tmp0 = snil, *tmp1 = snil;
	if(alist == nil || alist->type != PAIR)
		return makeerror(1,0,"assq item : EQABLE-SEXPRESSION alist : ASSOC-LIST => sexpression");
	tmp0 = alist;
	while(tmp0 != snil)
	{
		tmp1 = car(tmp0);
		if(tmp1->type != PAIR)
			return makeerror(1,0,"ASSOCI-LIST : (EQABLE-SEXPRESSION SEXPRESSION)*");
		if(eqp(car(tmp1),item) == strue)
			return tmp1;
		tmp0 = cdr(tmp0);
	}
	return nil;	
}
SExp *
list(int n, ...)
{
	va_list ap;
	SExp *ret = tconcify(snil);
	va_start(ap,n);
	for(;n;n--)
		tconc(ret,va_arg(ap,SExp *));
	va_end(ap);
	return mcar(ret);
}
SExp *
vector(int n, ...)
{
	va_list ap;
	int i = 0;
	SExp *ret = makevector(n,nil);
	va_start(ap,n);
	for(;i < n;i++)
		ret->object.vec[i] = va_arg(ap,SExp *);
	va_end(ap);
	return ret;
}
void
register_procedure(SExp *(*fn)(SExp *,Symbol *),char *name,int arity, Symbol *e)
{
	SExp *t = nil;
	t = (SExp *)hmalloc(sizeof(SExp));
	t->type = PROCEDURE;
	//t->object.procedure = fn;
	t->object.procedure = (void *)fn;
	add_env(e,name,t);
}
int
gc_init()
{
	GC_INIT();
	return 1;
}
int
_igcd(int a, int b)
{
	int t = 0;
	if(a == 0)
		return b;
	while(b != 0)
	{
		t = b;
		b = a % b;
		a = t;
	}
	return a;
}
Symbol *
init_env()
{
	/*static SExp *snil = nil, *sfalse = nil, *strue = nil, *ssucc = nil, *sunsucc = nil, *mem_err = nil; */
	Symbol *tl_env = nil;
	snil = (SExp *)hmalloc(sizeof(SExp));
	if(snil == nil)
	{
		printf("[-] snil == nil...\n");
		return;
	}
	//LINE_DEBUG;
	snil->type = NIL;
	sfalse = (SExp *)hmalloc(sizeof(SExp));
	sfalse->type = BOOL;
	sfalse->object.c = 0;
	strue = (SExp *)hmalloc(sizeof(SExp));
	strue->type = BOOL;
	strue->object.c = 1;
	ssucc = (SExp *)hmalloc(sizeof(SExp));
	ssucc->type = GOAL;
	ssucc->object.c = 1;
	sunsucc = (SExp *)hmalloc(sizeof(SExp));
	sunsucc->type = GOAL;
	sunsucc->object.c = 0;
	mem_err = snil;
	pinf = (SExp *)hmalloc(sizeof(SExp));
	pinf->type = NUMBER;
	pinf->object.n = (Number *)hmalloc(sizeof(Number));
	pinf->object.n->type = REAL;
	pinf->object.n->nobject.real = (double)0x7f800000; /* positive infinity */
	ninf = (SExp *)hmalloc(sizeof(SExp));
	ninf->type = NUMBER;
	ninf->object.n = (Number *)hmalloc(sizeof(Number));
	ninf->object.n->type = REAL;
	ninf->object.n->nobject.real = (double)0xff800000; /* negative infinity */
	qnan = (SExp *)hmalloc(sizeof(SExp));
	qnan->type = NUMBER;
	qnan->object.n = (Number *)hmalloc(sizeof(Number));
	qnan->object.n->type = REAL;
	qnan->object.n->nobject.real = (double)0x7fffffff; /* Quiet NaN */
	snan = (SExp *)hmalloc(sizeof(SExp));
	snan->type = NUMBER;
	snan->object.n = (Number *)hmalloc(sizeof(Number));
	snan->object.n->type = REAL;
	snan->object.n->nobject.real = (double)0x7fbfffff; /* Signalling NaN */
	fake_rpar = (SExp *)hmalloc(sizeof(SExp));
	fake_rpar->type = NIL;
	fake_rsqr = (SExp *)hmalloc(sizeof(SExp));
	fake_rsqr->type = NIL;
	fake_rcur = (SExp *)hmalloc(sizeof(SExp));
	fake_rcur->type = NIL;
	seof = (SExp *)hmalloc(sizeof(SExp));
	seof->type = SEOF;
	svoid = (SExp *)hmalloc(sizeof(SExp));
	svoid->type = SVOID;
	//LINE_DEBUG;
	//LINE_DEBUG;
	tl_env = (Symbol *)hmalloc(sizeof(Symbol));
	tl_env->data = (Window *)hmalloc(sizeof(Window)); // 64 initial "windows"
	//LINE_DEBUG;
	tl_env->cur_offset = 0;
	tl_env->cur_size = 64;
	//LINE_DEBUG;
	tl_env->data->env = (Trie *)hmalloc(sizeof(Trie)); // initial "window"
	tl_env->data->next = nil;
	//LINE_DEBUG;
	tl_env->snil = snil;
	tl_env->svoid = svoid;
	tl_env->seof = seof;
	tl_env->strue = strue;
	tl_env->sfalse = sfalse;
	tl_env->ssucc = ssucc;
	tl_env->sunsucc = sunsucc;
	//LINE_DEBUG;
	add_env(tl_env,"car",makeprimitive(OPCAR,"car",0));
	add_env(tl_env,"cdr",makeprimitive(OPCDR,"cdr",0));
	add_env(tl_env,"cons",makeprimitive(OPCONS,"cons",0));
	add_env(tl_env,"quote",makeprimitive(OPQUOTE,"quote",1));
	add_env(tl_env,"length",makeprimitive(OPLENGTH,"length",0));
	add_env(tl_env,"def",makeprimitive(OPDEF,"def",1));
	add_env(tl_env,"+",makeprimitive(OPPLUS,"+",0));
	add_env(tl_env,"exact?",makeprimitive(OPEXACT,"exact?",0));
	add_env(tl_env,"inexact?",makeprimitive(OPINEXACT,"inexact?",0));
	add_env(tl_env,"real?",makeprimitive(OPREAL,"real?",0));
	add_env(tl_env,"integer?",makeprimitive(OPINTEGER,"integer?",0));
	add_env(tl_env,"complex?",makeprimitive(OPCOMPLEX,"complex?",0));
	add_env(tl_env,"rational?",makeprimitive(OPRATIONAL,"rational?",0));
	add_env(tl_env,"numerator",makeprimitive(OPNUM,"numerator",0));
	add_env(tl_env,"denomenator",makeprimitive(OPDEN,"denomenator",0));
	add_env(tl_env,"*",makeprimitive(OPMULT,"*",0));
	add_env(tl_env,"type",makeprimitive(OPTYPE,"type",0));
	add_env(tl_env,"-",makeprimitive(OPSUB,"-",0));
	add_env(tl_env,"/",makeprimitive(OPDIV,"/",0));
	add_env(tl_env,"gcd",makeprimitive(OPGCD,"gcd",0));
	add_env(tl_env,"lcm",makeprimitive(OPLCM,"lcm",0));
	add_env(tl_env,"ceil",makeprimitive(OPCEIL,"ceil",0));
	add_env(tl_env,"floor",makeprimitive(OPFLOOR,"floor",0));
	add_env(tl_env,"truncate",makeprimitive(OPTRUNCATE,"truncate",0));
	add_env(tl_env,"round",makeprimitive(OPROUND,"round",0));
	add_env(tl_env,"inexact->exact",makeprimitive(OPIN2EX,"inexact->exact",0));
	add_env(tl_env,"eq?",makeprimitive(OPEQ,"eq?",0));
	add_env(tl_env,"<",makeprimitive(OPLT,"<",0));
	add_env(tl_env,">",makeprimitive(OPGT,">",0));
	add_env(tl_env,"<=",makeprimitive(OPLTE,"<=",0));
	add_env(tl_env,">=",makeprimitive(OPGTE,">=",0));
	add_env(tl_env,"=",makeprimitive(OPNUMEQ,"=",0));
	add_env(tl_env,"quotient", makeprimitive(OPQUOTIENT,"quotient",0));
	add_env(tl_env,"modulo", makeprimitive(OPMOD,"modulo",0));
	add_env(tl_env,"remainder",makeprimitive(OPREMAINDER,"remainder",0));
	add_env(tl_env,"set!",makeprimitive(OPSET,"set!",1)); /* (set! x '(1 2 3)) ; should we break w/ Scheme again? (set! 'x '(1 2 3)) */
	add_env(tl_env,"fn",makeprimitive(OPLAMBDA,"fn",1));
	add_env(tl_env,"&",makeprimitive(OPAND,"&",0));
	add_env(tl_env,"|",makeprimitive(OPOR,"|",0));
	add_env(tl_env,"^",makeprimitive(OPXOR,"^",0));
	add_env(tl_env,"~",makeprimitive(OPNEG,"~",0));
	add_env(tl_env,"list",makeprimitive(OPLIST,"list",0));
	add_env(tl_env,"vector",makeprimitive(OPVECTOR,"vector",0));
	add_env(tl_env,"make-vector",makeprimitive(OPMKVEC,"make-vector",0));
	add_env(tl_env,"make-string",makeprimitive(OPMKSTRING,"make-string",0));
	add_env(tl_env,"string",makeprimitive(OPSTRING,"string",0));
	add_env(tl_env,"append",makeprimitive(OPAPPEND,"append",0));
	add_env(tl_env,"first",makeprimitive(OPFIRST,"first",0));
	add_env(tl_env,"rest",makeprimitive(OPREST,"rest",0));
	add_env(tl_env,"ccons",makeprimitive(OPCCONS,"ccons",0));
	add_env(tl_env,"nth",makeprimitive(OPNTH,"nth",0));
	add_env(tl_env,"keys",makeprimitive(OPKEYS,"keys",0));
	add_env(tl_env,"partial-key?",makeprimitive(OPPARTIAL,"partial-key?",0));
	add_env(tl_env,"cset!",makeprimitive(OPCSET,"cset!",0));
	add_env(tl_env,"empty?",makeprimitive(OPEMPTY,"empty?",0));
	add_env(tl_env,"define-macro",makeprimitive(OPDEFMACRO,"define-macro",1));
	add_env(tl_env,"gensym",makeprimitive(OPGENSYM,"gensym",0));
	add_env(tl_env,"imag-part",makeprimitive(OPIMAG,"imag-part",0));
	add_env(tl_env,"real-part",makeprimitive(OPREALP,"real-part",0));
	add_env(tl_env,"make-rectangular",makeprimitive(OPMKRECT,"make-rectangular",0));
	add_env(tl_env,"make-polar",makeprimitive(OPMKPOL,"make-polar",0));
	add_env(tl_env,"magnitude",makeprimitive(OPMAG,"magnitude",0));
	add_env(tl_env,"argument",makeprimitive(OPIMAG,"argument",0));
	add_env(tl_env,"conjugate!",makeprimitive(OPCONJBANG,"conjugate!",0));
	add_env(tl_env,"conjugate",makeprimitive(OPCONJ,"conjugate",0));
	add_env(tl_env,"polar->rectangular",makeprimitive(OPPOLREC,"polar->rectangular",0));/* given two real args, returns a rectangular complex */
	add_env(tl_env,"rectangular->polar",makeprimitive(OPRECPOL,"rectangular->polar",0));/* given a complex, converts type... */
	add_env(tl_env,"sin",makeprimitive(OPSIN,"sin",0));
	add_env(tl_env,"cos",makeprimitive(OPCOS,"cos",0));
	add_env(tl_env,"tan",makeprimitive(OPTAN,"tan",0));
	add_env(tl_env,"asin",makeprimitive(OPASIN,"asin",0));
	add_env(tl_env,"acos",makeprimitive(OPACOS,"acos",0));
	add_env(tl_env,"atan",makeprimitive(OPATAN,"atan",0));
	add_env(tl_env,"atan2",makeprimitive(OPATAN2,"atan2",0));
	add_env(tl_env,"cosh",makeprimitive(OPCOSH,"cosh",0));
	add_env(tl_env,"sinh",makeprimitive(OPSINH,"sinh",0));
	add_env(tl_env,"tanh",makeprimitive(OPTANH,"tanh",0));
	add_env(tl_env,"exp",makeprimitive(OPEXP,"exp",0));
	add_env(tl_env,"ln",makeprimitive(OPLN,"ln",0));
	add_env(tl_env,"abs",makeprimitive(OPABS,"abs",0));
	add_env(tl_env,"sqrt",makeprimitive(OPSQRT,"sqrt",0));
	add_env(tl_env,"exp2",makeprimitive(OPEXP2,"exp2",0));
	add_env(tl_env,"expm1",makeprimitive(OPEXPM1,"expm1",0));
	add_env(tl_env,"log2",makeprimitive(OPLOG2,"log2",0));
	add_env(tl_env,"log10",makeprimitive(OPLOG10,"log10",0));
	add_env(tl_env,"<<",makeprimitive(OPSHL,"<<",0));
	add_env(tl_env,">>",makeprimitive(OPSHR,">>",0));
	add_env(tl_env,"begin",makeprimitive(OPBEGIN,"begin",1));
	add_env(tl_env,"define-syntax",makeprimitive(OPDEFSYN,"define-syntax",1));
	add_env(tl_env,"string-append",makeprimitive(OPSTRAP, "string-append",0));
	add_env(tl_env,"apply",makeprimitive(OPAPPLY,"apply",0));
	add_env(tl_env,"assq",makeprimitive(OPASSQ,"assq",0));
	add_env(tl_env,"defrec",makeprimitive(OPDEFREC,"defrec",1));
	add_env(tl_env,"set-rec!",makeprimitive(OPSETREC,"set-rec!",1));
	add_env(tl_env,"dict",makeprimitive(OPDICT,"dict",0));
	add_env(tl_env,"make-dict",makeprimitive(OPMKDICT,"make-dict",0));
	add_env(tl_env,"dict-has?",makeprimitive(OPDICHAS,"dict-has?",0));
	add_env(tl_env,"clone-environment",makeprimitive(OPCLONENV,"clone-environment",0));
	add_env(tl_env,"bind-environment",makeprimitive(OPDEFENV,"bind-environment",1));
	add_env(tl_env,"set-environment!",makeprimitive(OPSETENV,"set-environment!",1));
	add_env(tl_env,"default-environment",makeprimitive(OPDEFAULTENV,"default-environment",0));
	add_env(tl_env,"null-environment",makeprimitive(OPNULLENV,"null-environment",0));
	add_env(tl_env,"coerce",makeprimitive(OPCOERCE,"coerce",0));
	add_env(tl_env,"error",makeprimitive(OPERROR,"error",0));
	add_env(tl_env,"cupdate",makeprimitive(OPCUPDATE,"cupdate",0));
	add_env(tl_env,"cslice",makeprimitive(OPCSLICE,"cslice",0));
	add_env(tl_env,"tconc!",makeprimitive(OPTCONC,"tconc!",0));
	add_env(tl_env,"make-tconc",makeprimitive(OPMKTCONC,"make-tconc",0));
	add_env(tl_env,"tconc-list",makeprimitive(OPTCONCL,"tconc-list",0));
	add_env(tl_env,"tconc->pair",makeprimitive(OPT2P,"tconc->pair",0));
	add_env(tl_env,"tconc-splice!",makeprimitive(OPTCONCSPLICE,"tconc-splice!",0));
	add_env(tl_env,"if",makeprimitive(OPIF,"if",1));
	add_env(tl_env,"eval",makeprimitive(OPEVAL,"eval",0));
	add_env(tl_env,"meta",makeprimitive(OPMETA,"META",0));
	add_env(tl_env,"reset",makeprimitive(OPRESET,"reset",0));
	add_env(tl_env,"shift",makeprimitive(OPSHIFT,"shift",0));
	add_env(tl_env,"call/cc",makeprimitive(OPCALLCC,"call/cc",0));
	add_env(tl_env,"current-tick",makeprimitive(OPCURTICK,"current-tick",0));
	//LINE_DEBUG;
	/* seed the random system*/
	srandom(time(nil));
	return tl_env;
}
void
clean_env()
{
	int i = 0; 
	/*free(heap);
	free(free_list);*/
	while(0) {
		i++;
	}
}
SExp *
makechar(char c)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = CHAR;
	ret->object.c = c;
	ret->metadata = nil;
	return ret;
}
SExp *
makeport(FILE *fp,char *src, int proto, int bind, char *mode)
{
	SExp *ret = snil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = PORT;
	ret->metadata = nil;
	ret->object.p = (Port *)hmalloc(sizeof(Port));
	PTYPE(ret) = PFILE;
	FILEPORT(ret) = fp;
	FILEADDRESS(ret) = hstrdup(src);
	PROTONUMBER(ret) = proto;
	NETBIND(ret) = bind;
	strncpy(FILEMODE(ret),mode,3);
	return ret;
}
SExp *
makenumber(int t)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = NUMBER;
	ret->metadata = nil;
	ret->object.n = (Number *)hmalloc(sizeof(Number));
	ret->object.n->type = t;
	if(t == INTEGER)
		set_int(ret,0);
	return ret;
}
SExp *
makeinteger(int s)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = NUMBER;
	ret->metadata = nil;
	ret->object.n = (Number *)hmalloc(sizeof(Number));
	ret->object.n->type = INTEGER;
	ret->object.n->nobject.z = s;
	return ret;
}
SExp *
makereal(double f)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = NUMBER;
	ret->metadata = nil;
	ret->object.n = (Number *)hmalloc(sizeof(Number));
	ret->object.n->type = REAL;
	ret->object.n->nobject.real = f;
	return ret;
}
SExp *
makerational(int n,int d)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = NUMBER;
	ret->metadata = nil;
	ret->object.n = (Number *)hmalloc(sizeof(Number));
	ret->object.n->type = RATIONAL;
	NUM(ret) = n;
	DEN(ret) = d;
	return ret;
}
SExp *
makecomplex(double r,double i)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = NUMBER;
	ret->metadata = nil;
	ret->object.n = (Number *)hmalloc(sizeof(Number));
	ret->object.n->type = COMPLEX;
	CEREAL(ret) = r;
	IMMAG(ret) = i;
	return ret;
}
SExp *
makeatom(char *s)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	if(ret == nil)
	{
		printf("hmalloc returned nil! PANIC!\n");
		quit_note = 1;
		return nil;
	}
	ret->type = ATOM;
	ret->metadata = nil;
	set_str(ret,s);
	return ret;
}
SExp *
makestring_v(int size, char fill)
{
	SExp *ret = nil;
	int iter = 0;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = STRING;
	ret->metadata = nil;
	ret->object.str = (char *)hmalloc(sizeof(char) * size + 1);
	if(fill != nul)
	{
		for(;iter < size;iter++)
			ret->object.str[iter] = fill;
		ret->object.str[iter] = nul;
	}
	ret->length = size;
	return ret;
}
SExp *
makestring(const char *s)
{
	SExp *ret = nil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = STRING;
	ret->metadata = nil;
	set_str(ret,s);
	ret->length = strlen(s);
	return ret;
}
SExp *
makeerror(int t, int s, char *f)
{
	SExp *ret = snil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = ERROR;
	ret->metadata = nil;
	ret->object.error.source = (s & 0xFF);
	ret->object.error.level = t;
	ret->object.error.message = hstrdup(f);
	return ret;	
}
SExp *
makeprimitive(int n, char *s, int f)
{
	SExp *ret = snil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = PRIM;
	ret->metadata = nil;
	ret->object.primitive.evalp = (char) f & 0xFF;
	ret->object.primitive.name = hstrdup(s);
	ret->object.primitive.num = n;
	return ret;
}
SExp *
makevector(int n, SExp *fill)
{
	SExp *ret = snil;
	int iter = 0;
	ret = (SExp *)hmalloc(sizeof(SExp));
	if(n > 0)
	{
		ret->object.vec = (SExp **)hmalloc(sizeof(SExp *) * n);
		if(fill != nil) /* for efficiency; don't fill a vector not used by the user */
			for(;iter < n;iter++)
				ret->object.vec[iter] = fill;
	}
	ret->length = n;
	ret->type = VECTOR;
	ret->metadata = nil;
	return ret;
}
SExp *
makedict()
{
	SExp *ret = snil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = DICT;
	ret->metadata = nil;
	ret->object.dict = (Trie *)hmalloc(sizeof(Trie));
	ret->object.dict->key = nul;
	ret->object.dict->n_len = 0;
	ret->object.dict->n_cur = 0;
	ret->object.dict->nodes = nil;
	return ret;
}
SExp *
makeenv(Symbol *e)
{
	SExp *ret = snil;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = ENVIRONMENT;
	ret->metadata = nil;
	ret->object.foreign = (void *) e;
	return ret;
}
char *
hstrdup(const char *s)
{
	char *ret = nil;
	int l = 0, i = 0;
	if(s == nil)
		return nil;
	l = strlen(s);
	if((ret = (char *)hmalloc(sizeof(char) * l + 1)) == nil)
		return nil;
	for(;i < l;i++)
		ret[i] = s[i];
	ret[i] = nul;
	return ret;
}
SExp *
symlookup(char *str, Symbol *env)
{
	int idx = 0,len = 0;
	SExp *d = nil;
	Window *cur = nil;
	Trie *hd = nil;
	//printf("%s:%d\n",__FUNCTION__,__LINE__);
	if(env == nil || env->data->env == nil)
		return nil;
	//printf("%s:%d\n",__FUNCTION__,__LINE__);
	cur = env->data;
	while(cur != nil)
	{
		//printf("SYMLOOKUP loop\n");
		d = trie_get(str,cur->env);
		if(d != nil)
			break;
		cur = cur->next;
	}
	/*printf("%s:%d\n",__FUNCTION__,__LINE__);
	if(d != nil)
	{
		printf("d == ");
		princ(d);
		printf("\n");
	}
	else
		printf("d == nil\n");*/
	return d;
}
int
new_window(Symbol *inp)
{
	Window *w = nil;
	w = (Window *)hmalloc(sizeof(Window));
	w->env = (Trie *)hmalloc(sizeof(Trie));
	w->next = inp->data;
	inp->data = w;
	return 1;
}
int
close_window(Symbol *inp)
{
	if(inp != nil && inp->data != nil)
	{
		inp->data = inp->data->next;
		return 1;
	}
	return 0;
}
Symbol *
shallow_clone_env(Symbol *src)
{
	Symbol *ret = nil;
	int i = 0;
	if(src == nil)
		return nil;
	ret = (Symbol *)hmalloc(sizeof(Symbol));
	ret->cur_offset = src->cur_offset;
	ret->cur_size = src->cur_size;
	ret->data = src->data;
	ret->snil = src->snil;
	ret->svoid = src->svoid;
	ret->strue = src->strue;
	ret->sfalse = src->sfalse;
	ret->ssucc = src->ssucc;
	ret->sunsucc = src->sunsucc;
	ret->seof = src->seof;
	ret->tick = src->tick;
	return ret;	
}
Symbol *
add_env(Symbol *inp, char *name, SExp *data)
{
	Trie *hd = nil;
	SExp *d = nil;
	if(inp != nil && inp->data != nil)
	{
		hd = inp->data->env;
		trie_put(name,data,hd);
		return inp;
	}
	return nil;
}
SExp *
trie_put(char *str, SExp *value, Trie *head)
{
	Trie *tmp = nil;
	int iter = 0;
	if(strlen(str) == 0)
	{
		head->data = value;
		return strue;
	}
	if(head->nodes == nil)
	{
		head->nodes = (Trie **)hmalloc(sizeof(Trie *) * 4); /* allocate 4, use one */
		head->n_cur = 1;
		head->n_len = 4;
		head->nodes[0] = trie_alloc(str,value);
		return strue;
	}
	for(iter = 0;iter < head->n_cur;iter++)
	{
		if(head->nodes[iter]->key == str[0])
		{
			trie_put(&str[1],value,head->nodes[iter]);
			return strue;
		}
	}
	/* ok, so the child list is not empty, and we didn't find a matching key, so
	** we have to check now if we have room left in nodes or if it needs to be re-
	** alloc'd, and talloc the remainder of the key...
	*/
	iter = head->n_cur;
	if(iter < head->n_len)
	{
		head->nodes[iter] = trie_alloc(str,value);
		head->n_cur++;
	}
	else
	{
		head->nodes = hrealloc(head->nodes,sizeof(Trie **) * (head->n_len + 4));
		head->nodes[iter] = trie_alloc(str,value);
		head->n_cur++;
		head->n_len += 4;
	}
	return strue;
}
SExp *
trie_get(char *key, Trie *head)
{
	int iter = 0;
	if(strlen(key) == 0)
		return head->data;
	if(head->nodes == nil)
		return nil;
	for(;iter < head->n_cur;iter++)
		if(head->nodes[iter]->key == key[0])
			return 	trie_get(&key[1],head->nodes[iter]);
	return nil;
}
SExp *
trie_keys(Trie *hd, SExp *thus_far) // pass in a tconc object that can be cloned when you want to return...
{
	int iter = 0;
	SExp *ret = tconcify(snil), *tmp0 = nil, *tmp1 = nil;
	//printf("Made it to trie_keys\n");
	if(hd == nil)
		return snil;
	//printf("hd == nil: %d\n",hd == nil);
	if(hd->data != nil)
	{
		//printf("\tMade it to hd->data != nil!\n");
		tmp0 = fstring(mcar(thus_far));
		if(tmp0->type == ERROR)
			return tmp0;
		tconc(thus_far,makechar(hd->key));
		tconc(ret,fstring(mcar(thus_far)));
	}
	//printf("\tmade it to 0\n");
	if(hd->nodes != nil)
	{
		//printf("\tmade it to 1\n");
		for(;iter < hd->n_cur; iter++)
		{
			// capture the current state of thus_far in tmp
			// since tconc is destructive...
			tmp1 = mcar(thus_far);
			tmp0 = tconcify(snil);
			/*printf("\tMade it to 2\nthus_far(%s) == ",typenames[thus_far->type]);
			princ(thus_far);
			printf("\n");*/
			while(tmp1 != snil)
			{
				/*printf("tmp1 == nil? %d\n",tmp1 == nil);
				printf("\t\tLooping near 3\ntmp1 == ");
				princ(tmp1);*/
				tconc(tmp0,car(tmp1));
				//printf("\n\t\tLooping near 4\n");
				tmp1 = cdr(tmp1);
			}
			/*printf("thus_far == ");
			princ(thus_far);
			printf("\nret == ");
			princ(ret);
			printf("\n");*/
			if(hd->key != 0 && hd->data == nil)
			{
				tconc(tmp0,makechar(hd->key));
				//ret = cons(trie_keys(hd->nodes[iter],tmp0),ret);
				tmp1 = trie_keys(hd->nodes[iter],tmp0);
				/*while(tmp1 != snil)
				{
					if(mcar(tmp1)->type == ATOM)
						break;
					tmp1 = cdr(tmp1);
				}*/
				/*printf("ret before tconc_splice: ");
				princ(ret);
				printf("\nret after tconc_splice: ");*/
				tconc_splice(ret,tmp1);
				/*princ(ret);
				printf("\n");*/
			}
			else
			{
				tconc_splice(ret,trie_keys(hd->nodes[iter],tmp0));
			}
		}
	}
	/*printf("Ret == ");
	princ(ret);
	printf("\n");*/
	return ret;
}
SExp *
trie_values(Trie *head)
{
	return sfalse;
}
SExp *
trie_pairs(Trie *head)
{
	return sfalse;
}
Trie *
trie_alloc(char *src, SExp *value)
{
	Trie **ret = nil;
	int iter = 0, len = strlen(src);
	if(src == nil || len == 0)
		return nil;
	ret = (Trie **)hmalloc(sizeof(Trie *) * len);
	for(;iter < len;iter++)
	{
		ret[iter] = (Trie *)hmalloc(sizeof(Trie));
		ret[iter]->n_len = 1;
		ret[iter]->n_cur = 1;
		ret[iter]->key = src[iter];
		ret[iter]->data = nil;
		ret[iter]->nodes = (Trie **)hmalloc(sizeof(Trie *));
	}
	ret[iter - 1]->data = value;
	for(iter = 0;iter < len - 1;iter++)
		ret[iter]->nodes[0] = ret[iter + 1];
	ret[iter]->nodes = nil;
	return ret[0];
}
void
trie_walk(Trie *head, int level)
{
	int iter = 0;
	Trie *child;
	for(;iter < level; iter++)
		printf("   ");
	printf("%c",head->key);
	if(head->data != nil)
	{
		printf(": ");
		princ(head->data);
	}
	printf("\n");
	if(head->nodes != nil)
	{
		printf("head->n_cur == %d\n",head->n_cur);
		for(iter = 0;iter < head->n_cur;iter++)
			trie_walk(head->nodes[iter],level + 1);
	}
}
SExp *
trie_partial(Trie *head, char *key, int len)
{
	int i = 0;
	if(len == 0 && head->data != nil)
		return strue;
	if(head->nodes != nil)
	{
		for(;i < head->n_cur;i++)
			if(head->nodes[i]->key == key[0])
				return trie_partial(head->nodes[i],&key[1],len + 1);
		if(len > 0)
			return makeinteger(len);
		return sfalse;
	}
	if(len > 0)
		return makeinteger(len);
	return sfalse;
}
SExp *
trie_hasp(Trie *head, char *key)
{
	SExp *t = nil;
	if(head == nil || key == nil)
		return sfalse;
	t = trie_get(key,head);
	if(t == nil)
		return sfalse;
	return strue;
}
SExp *
newline(SExp *s, Symbol *env)
{
	int itmp = pairlength(s);
	SExp *port = snil;
	//LINE_DEBUG;
	switch(itmp)
	{
		case 0:
			printf("\n");
			break;
		case 1:
			port = car(s);
			if(port->type != PORT)
				return makeerror(2,0,"newline's *p* must be bound to a PORT");
			fprintf(FILEPORT(port),"\n");
			break;
		default:
			return makeerror(2,0,"newline [p : PORT] => NIL");
	}
	//LINE_DEBUG;
	return svoid;
}
SExp *
f_load(SExp *s, Symbol *env)
{
	FILE *fdin = nil;
	SExp *ret = env->snil;
	/*printf("Rigors (in f_load): \n");
	if(ret != nil)
	{
		printf("ret->type == %s\n",typenames[ret->type]);
		printf("ret == snil? %d\n",(ret == snil ? 1 : 0));
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
		//llprinc(ret,stderr,1);
		if(ret->type == ERROR)
			break;
		else if(ret == seof)
			break;
		ret = lleval(ret,env);
		if(quit_note)
			break;
		if(ret->type == ERROR)
			break;
	}
	if(ret->type == ERROR)
		return ret;
	return env->strue;	
}
SExp *
f_princ(SExp *s, Symbol *env)
{
	int i = pairlength(s);
	SExp *t = env->snil, *f = env->snil;
	if(i < 1 || i > 2)
		return makeerror(2,0,"display expects at least one argument, and no more than two: display o : SEXPRESSION [p : PORT] => SEXRESSION");
	f = car(s);
	if(i == 1)
		llprinc(f,stdout,0); /* should look up what *current-output-port* is set to... */
	else 
	{
		t = car(cdr(s));
		if(t->type != PORT)
			return makeerror(2,0,"display's p argument must be of type PORT");
		llprinc(f,FILEPORT(t),0);
	}
	return f;
}

void
princ(SExp *s)
{
	/* Need to make this accept a Digamma * object, and use the 
	 * current stdout (e->stdout) for llprinc, so that set-current-output &
	 * friends have actual use...
	 */
	llprinc(s,stdout,0);
}
void
llprinc(SExp *s, FILE *fd, int mode)
{
	SExp *fst = snil, *rst = snil, *tmp = snil;
	int iter = 0;
	if(s == nil)
		return ;
	switch(s->type)
	{
		case PAIR:
			fprintf(fd,"(");
			rst = s;
			while(rst != snil)
			{
				if(rst->type != NIL && mcdr(rst)->type != PAIR)
				{
					llprinc(car(rst),fd,mode);
					if(mcdr(rst)->type != NIL)
					{
						fprintf(fd," . ");
						llprinc(cdr(rst),fd,mode);
					}
					break;
				}
				else
				{
					fst = car(rst);
					llprinc(fst,fd,mode);
					rst = cdr(rst);
					if(rst->type != PAIR && rst->type != NIL)
						fprintf(fd," . ");
					else
						fprintf(fd," ");
				}
			}
			fprintf(fd,")");
			break;
		case TCONC:
			llprinc(mcar(s),fd,mode);
			break;
		case PRIM:
			fprintf(fd,"#<\"%s\" %d>", s->object.primitive.name, s->object.primitive.num);
			break;
		case NUMBER:
			switch(s->object.n->type)
			{
				case INTEGER:
					fprintf(fd,"%d", s->object.n->nobject.z);
					break;
				case REAL:
					fprintf(fd,"%.32g", s->object.n->nobject.real);
					break;
				case RATIONAL:
					fprintf(fd,"%d/%d", s->object.n->nobject.rational.num,s->object.n->nobject.rational.den);
					break;
				case COMPLEX:
					if(s->object.n->nobject.complex.r >= 0.0)
						fprintf(fd,"+%f",s->object.n->nobject.complex.r);
					else
						fprintf(fd,"%f",s->object.n->nobject.complex.r);
					if(s->object.n->nobject.complex.i >= 0.0)
						fprintf(fd,"+%fi",s->object.n->nobject.complex.i);
					else
						fprintf(fd,"%fi",s->object.n->nobject.complex.i);
					break;
			}
			break;
		case GOAL:
			if(s == ssucc)
				fprintf(fd,"#s");
			else 
				fprintf(fd,"#u");
			break;
		case BOOL:
			if(s == strue)
				fprintf(fd,"#t");
			else
				fprintf(fd,"#f");
			break;
		case CHAR:
			if(mode)
			{
				switch(s->object.c)
				{
					case ' ':
						fprintf(fd,"#\\space");
						break;
					case '\n':
						fprintf(fd,"#\\linefeed");
						break;
					case '\r':
						fprintf(fd,"#\\carriage");
						break;
					case '\t':
						fprintf(fd,"#\\tab");
						break;
					case '\v':
						fprintf(fd,"#\\vtab");
						break;
					case 0x07:
						fprintf(fd,"#\\bell");
						break;
					case 0x00:
						fprintf(fd,"#\\nul");
						break;
					default:
						fprintf(fd,"#\\%c",s->object.c);
						break;
				}
			}
			else
				fprintf(fd,"%c",s->object.c);
			break;
		case VECTOR:
			fprintf(fd,"[ ");
			for(;iter < s->length;iter++)
			{
				llprinc(s->object.vec[iter],fd,mode);
				fprintf(fd," ");
			}
			fprintf(fd,"]");
			break;
		case KEY:
			fprintf(fd,":");
		case ATOM:
			fprintf(fd,"%s",s->object.str);
			break;
		case STRING:
			// write mode should escape characters like \t & \n
			if(mode)
			{
				fprintf(fd,"\"");
				for(;iter < s->length;iter++)
				{
					switch(s->object.str[iter])
					{
						case '\"':
							fprintf(fd,"\\\"");
							break;
						case '\\':
							fprintf(fd,"\\\\");
							break;
						case '\n':
							fprintf(fd,"\\n");
							break;
						case '\t':
							fprintf(fd,"\\t");
							break;
						case '\r':
							fprintf(fd,"\\r");
							break;
						case '\v':
							fprintf(fd,"\\v");
							break;
						case '\a':
							fprintf(fd,"\\a");
							break;
						case '\b':
							fprintf(fd,"\\b");
							break;
						case '\0':
							fprintf(fd,"\\0");
							break;	
						default:	
							fprintf(fd,"%c",s->object.str[iter]);
							break;
					}	
				}
				fprintf(fd,"\"");
			}
			else
				fprintf(fd,"%s",s->object.str);
			break;
		case NIL:
			fprintf(fd,"()");
			break;
		case SEOF:
			fprintf(fd,"#e");
			break;
		case SVOID:
			fprintf(fd,"#v");
			break;
		default:
			fprintf(fd,"#<%s>",typenames[s->type]);
			break;
	}
}

int
lex(FILE *fdin, char **r)
{
  int c = 0, iter = 0, tmp = 0, state = 0, substate = 0;
	char *ret = *r;
	c = fgetc(fdin);
	//printf("lex start marker\n");
	while(!feof(fdin))
	{
			while(c == ' ' || c == '\t' || c == '\r' || c == '\n')
				c = fgetc(fdin);
			//printf("[!] top level loop: %d %d\n",state,substate);
			switch(state)
			{
				case 0: /* default state; attempt to figure out where to go... */
					if((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')
						state = 1;
					else if(c == '#')
						state = 2;
					else if(c == '"')
						state = 3;
					else if(c == '(')
						return TOK_LPAREN;
					else if(c == ')')
						return TOK_RPAREN;
					else if(c == '{')
						return TOK_LCURLY;
					else if(c == '}')
						return TOK_RCURLY;
					else if(c == '[')
						return TOK_LSQUAR;
					else if(c == ']')
						return TOK_RSQUAR;
					else if(c == '\'')
						return TOK_NQUOTE;
					else if(c == '`')
						return TOK_MQUOTE;
					else if(c == ',')
					{
						tmp = fgetc(fdin);
						if(tmp == '@')
							return TOK_SPLICE;
						ungetc(tmp,fdin);
						return TOK_UNQUOT;
					}
					else if(c == ';') /* better comment removal... */
					{
						while(c != '\n')
						{
							/* It's not necessarily true that a newline will
							 * exist at the end of a file, where a comment is...
							 */
							if(feof(fdin))
								return TOK_LEOF;
							c = fgetc(fdin);
						}
						state = 0;
						break;
					}
					else if(feof(fdin))
						return TOK_LEOF;
					else
					{
						ret[iter] = c;
						iter++;
						state = 4;
					}
					break;
				case 1:
          //printf("\t\t[!] Ok, we made it to case 1...\n");
					if((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')
					{
						/* some sort of number? Numbers must start with
						* [0-9] but can contain anynumber of other characters,
						* depending on type:
						* complex -> +3.4+5.6i
						*/
						if(c == '+' || c == '-')
						{
							tmp = fgetc(fdin);
							if(!(tmp >= '0' && tmp <= '9'))
							{
								/* we have a symbol most likely */
								ungetc(tmp,fdin);
								ungetc(c,fdin);
								state = 4;
								break;
							}
							ret[0] = c;
							c = tmp;
							iter = 1;
						}
						/* should make this stateful, so we can check that we're actually sending real numbers...
						 * e.g. +3.5-4.6i should lex fine, but under this scheme so would 3iiiiiiii2.0, which
						 * should be a symbol. Although it adds a bit of complexity, a state machine here would
						 * mean less processing afterwards (plus we can make the return type more granular: instead
						 * of simply returning TOK_NUMBER, we could return TOK_COMPLEX, TOK_INTEGER, &c.
						 * Should look at supporting BIGINTS & BigDecimals too...
						 */
            //printf("\t\t[!] Made it to substate switch...\n");
						substate = 0;
						//if(c == '.')
						//	substate = 2;
						while((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '/' || c == '.' || c == 'i' || c == 'e' || c == 'E')
						{
              //printf("\t\t[!] Looping around like mad in substate (%d)!\n",substate);
							switch(substate)
							{
								case 0: /* default case */
									if((c >= '0' && c <= '9') || c == '+' || c == '-')
										substate = 1;
									else if(c == '.')
									    substate = 2;
									else
									{
										substate = 99;
										state = 4; /* something other than a number... */
									}
									ret[iter] = c;
									iter++;
                  					//printf("\t\t[?] substate == %d, c == %c\n",substate,c);
									break;
								case 1: /* integer */
                 					//printf("\t\t[?] Did we make it to the integer case?\n");
									while(1)
									{
										c = fgetc(fdin);
                    					//printf("\t\t[!] I'm looping around forever in lex\n");
                    					ret[iter] = c;
										iter++;
										if(c == '.')
										{
											substate = 2;
											break;
										}
										else if(c == 'e' || c == 'E')
										  {
										    substate = 5;
										    break;
										  }
										else if(c == '/')
										{
											substate = 3;
											break;
										}
										else if(c == '+' || c == '-')
										{
											substate = 4;
											break;
										}
										else if(!issymdelim(c))
										{
											ungetc(c,fdin);
											iter--;
											break;
										}
										else if(!(c >= '0' && c <= '9'))
										{
											substate = 99;
											state = 4;
											break;
										}
									}
									if(substate != 1)
										break;
									ret[iter] = nul;
									return TOK_INT;
								case 2: /* real */
									/* we should have already had a '.', or an 'e' */
									while(1)
									{
										c = fgetc(fdin);
										ret[iter] = c;
										iter++;
										if(c == '+' || c == '-')
										{
											substate = 4;
											break;
										}
										else if(c == 'e' || c == 'E')
										  {
										    substate = 5;
										    break;
										  }
										else if(!issymdelim(c))
										{
											iter--;
											//printf("ret[iter - 1] == #\\%c\n",ret[iter - 1]);
											if(ret[iter - 1] == '.') /* single #\. input */
											{
												//printf("Made it?\n");
												ret[iter] = nul;
												ungetc(c,fdin);
												return TOK_SYMBOL;
											}
											ungetc(c,fdin);
											break;
										}
										else if(!(c >= '0' && c <= '9'))
										{
											state = 4;
											substate = 99;
											break;
										}
									}
									if(substate != 2)
										break;
									ret[iter] = nul;
									return TOK_REAL;
								case 3: /* rational */
									//printf("\t\t[!] made it to the rational substate...\n");
									while(1)
									{
										c = fgetc(fdin);
										ret[iter] = c;
										iter++;
										//printf("\t\t\t rational substate loop: c == %c\n",c);
										if(!issymdelim(c))
										{
											iter--;
											ungetc(c,fdin);
											break;
										}
										if(!(c >= '0' && c <= '9'))
										{
											state = 4;
											substate = 99;
											break;
										}
									}
									if(substate != 3)
										break;
									ret[iter] = nul;
									return TOK_RATIO;
									break;
								case 4: /* complex */
									/* Make a state machine out of this, so that we *know* we have a
									 * complex, not a symbol who matches the pattern...
									 */
									/* but wait a minute: by the time we get here, we should have seen [+|-][NUMBERS][+|-]
									 * there is no need to keep this as part of the game show...
									 */
									while(1)
									{
										c = fgetc(fdin);
										ret[iter] = c;
										iter++;
										if(!issymdelim(c))
										{
											iter--;
											ungetc(c,fdin);
											break;
										}
										else if(!((c >= '0' && c <= '9') || c == '.' || c == 'i'))
										{
											state = 4;
											substate = 99;
											break;
										}
									}
									if(substate != 4)
										break;
									ret[iter] = nul;
									return TOK_COMPL;
	      							case 5: /* cyantific notation */
								  while(1)
								    {
								      c = fgetc(fdin);
								      ret[iter] = c;
								      iter++;
								      if(!issymdelim(c))
									{
									  iter--;
									  ungetc(c,fdin);
									  break;
									}
								      else if(!((c >= '0' && c <= '9') || c == '-' || c == '+'))
									{
									  state = 4;
									  substate = 99;
									  break;
									}
								    }
								  if(substate != 5)
								    break;
								  ret[iter] = nul;
								  return TOK_REAL;
								default: /* some other state, probably 99 */
									break;
							}
							if(substate > 5)
								break;
							/*ret[iter] = c;
							c = fgetc(fdin);				
							iter++;*/
						}
					}
					break;
			case 2: /* hash object, #t #s #f #u #\character #|named_char */
				tmp = fgetc(fdin);
				/* I wonder if the return results for #t, #f, #s & #u should
				 * be constants. I mean, they are constant values, so perhaps this
				 * should return TOK_TRUE, TOK_FALSE, TOK_SUCC, TOK_UNSUCC...
				 * Looks good, implement it.
				 */
				/* add Scheme vector test:
				 * #(vector) & #N(vector) (for a vector of size N)
				 */
				switch(tmp)
				{
					case 'f':
					case 'F': /* #f */
						return TOK_FALSE;
					case 't':
					case 'T': /* #t */
						return TOK_TRUE;
					case 's':
					case 'S': /* #s */
						return TOK_SUCC;
					case 'u':
					case 'U': /* #u */
						return TOK_UNSUCC;
					case 'e':
					case 'E': /* #e, EOF literal */
						return TOK_LEOF;
					case 'v':
					case 'V':
						return TOK_LVOID;
					case '!':
						while(c != '\n') c = fgetc(fdin);
						state = 0;
						break;
					case '\\': /* character #\c */
						c = fgetc(fdin);
						if(c == ' ' || c == '\n' || c == '\t')
						{
							ret[0] = c;
							ret[1] = nul;
							return TOK_CHAR;
						}
						while(1)
						{
							ret[iter] = c;
							c = fgetc(fdin);
							iter++;
							if(c == '{' || c == '}' || c == ' ' || c == '(' || c == '\n' || c == '\t' || c == ')' || c == '[' || c == ']')
								break;
						}
						ungetc(c,fdin);
						ret[iter] = nul;
						return TOK_CHAR;
					case '|':
					  /* named character #|eof| */
						break;
					case ',':
						/* SRFI-10 */
						break;
					case 'x':
					case 'X':
						/* hex */
						while(1)
						{
							c = fgetc(fdin);
							ret[iter] = c;
							iter++;
							if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) 
							{
								iter--;
								ungetc(c,fdin);
								break;
							}
						}
						ret[iter] = nul;
						return TOK_HEX;
					case 'o':
					case 'O':
						/* octal */
						while(1)
						{
							c = fgetc(fdin);
							ret[iter] = c;
							iter++;
							if(!(c >= '0' && c <= '7'))
							{
								iter--;
								ungetc(c,fdin);
								break;
							}
						}
						ret[iter] = nul;
						return TOK_OCT;
					case 'b':
					case 'B':
						/* binary */
						while(1)
						{
							c = fgetc(fdin);
							ret[iter] = c;
							iter++;
							if(c != '0' && c != '1')
							{
								iter--;
								ungetc(c,fdin);
								break;
							}
						}
						ret[iter] = nul;
						return TOK_BIN;
					case EOF:
						return TOK_EOF;
					default: /* syntax error */
						return TOK_HERROR;
				}
				break;
			case 3: /* literal string */
				c = fgetc(fdin);
				while(c != '"')
				{
					if(feof(fdin))
						return TOK_SERROR;
					if(c == '\\') // make \" work :D
					{
						c = fgetc(fdin);
						if(feof(fdin))
							return TOK_SERROR;
						else if(c == 't')
							c = '\t';
						else if(c == 'n')
							c = '\n';
						else if(c == 'r')
							c = '\r';
						else if(c == 'v')
							c = '\v';
						else if(c == '0')
							c = '\0';
						else if(c == 'b')
							c = '\b';
						else if(c == 'a')
							c = '\a';
					}
					ret[iter] = c;
					c = fgetc(fdin);
					iter++;
				}
				ret[iter] = nul;
				return TOK_LITSTR;
			case 4:  /* symbol */
				c = fgetc(fdin);
				while(c != '"' && c != '\'' && c != '(' && c != ')' && c != '[' && c != ']' && c != '{' && c != '}' && c != ' ' && c != '\n' && c != '\t')
				{
					ret[iter] = c;
					c = fgetc(fdin);
					iter++;
				}
				ret[iter] = nul;
				ungetc(c,fdin);
				if(ret[0] == ':' || ret[iter - 1] == ':')
					return TOK_KEY;
				return TOK_SYMBOL;
		}
	}
	if(feof(fdin))
		return TOK_EOF;
}
SExp *
llread(FILE *fdin)
{
	int tok = 0, iter = 0, itmp = 0, cur_state = 0, state_stack[64] = {0}, sp = -1;
	SExp *ret = snil, *holder = snil, *tmp0 = snil, *tmp1 = snil, *tmp2 = snil, *read_stack[64] = {snil};
	static char *buf = nil;
	if(buf == nil)
		buf = (char *)hmalloc(sizeof(char) * 2048);
	tok = lex(fdin,&buf);
	while(1)
	{
		//printf("tok == %d\n",tok);
		switch(tok)
		{
			case TOK_TRUE:
				_read_return(strue);
			case TOK_FALSE:
				_read_return(sfalse);
			case TOK_SUCC:
				_read_return(ssucc);
			case TOK_UNSUCC:
				_read_return(sunsucc);
			case TOK_CHAR:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = CHAR;
				/* To Do:
				 * o check for invalid named characters
				 * o test the length, and if's == 1, skip these strcmps
				 * o support Unicode (UTF-8) hex characters...
				 */
				if(!strncasecmp(buf,"space",5))
					ret->object.c = ' ';
				else if(!strncasecmp(buf,"newline",7))
					ret->object.c = '\n';
				else if(!strncasecmp(buf,"linefeed",8))
					ret->object.c = '\n';
				else if(!strncasecmp(buf,"carriage",8))
					ret->object.c = '\r';
				else if(!strncasecmp(buf,"bell",4))
					ret->object.c = 0x07;
				else if(!strncasecmp(buf,"nul",3))
					ret->object.c = nul;
				else if(!strncasecmp(buf,"tab",3))
					ret->object.c = '\t';
				else if(!strncasecmp(buf,"vtab",3))
					ret->object.c = '\v';
				else
					ret->object.c = buf[0];
				_read_return(ret);
			case TOK_KEY:
				ret = (SExp *) hmalloc(sizeof(SExp));
				if(buf[0] == ':')
				{
					ret->object.str = hstrdup(&buf[1]);
					ret->length = strlen(&buf[1]);
				}
				else
				{
					itmp = strlen(buf);
					buf[itmp - 1] = nul; /* key: */
					ret->object.str = hstrdup(buf);
					ret->length = itmp - 1;
				}
				ret->type = KEY;
				_read_return(ret);
			case TOK_LITSTR:
			case TOK_SYMBOL:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->object.str = hstrdup(buf);
				if(tok == TOK_LITSTR)
					ret->type = STRING;
				else
					ret->type = ATOM;
				ret->length = strlen(buf);
				_read_return(ret);
			case TOK_HEX:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->nobject.z = atox(buf);
				ret->object.n->type = INTEGER;
				_read_return(ret);
			case TOK_OCT:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->nobject.z = atoo(buf);
				ret->object.n->type = INTEGER;
				_read_return(ret);
			case TOK_BIN:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->nobject.z = atob(buf);
				ret->object.n->type = INTEGER;
				_read_return(ret);
			case TOK_INT:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->nobject.z = atoi(buf);
				ret->object.n->type = INTEGER;
				_read_return(ret);
			case TOK_REAL:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->nobject.real = strtod(buf,nil);
				ret->object.n->type = REAL;
				_read_return(ret);
			case TOK_RATIO:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->type = RATIONAL;
				while(buf[iter] != '/') iter++;
				ret->object.n->nobject.rational.num = atoi(buf);
				ret->object.n->nobject.rational.den = atoi(&buf[iter + 1]);
				/* rationals should always in lowest terms */
				if((itmp = _igcd(ret->object.n->nobject.rational.num,ret->object.n->nobject.rational.den)) != 1)
				{
					while(itmp != 1) 
					{
						ret->object.n->nobject.rational.num /= itmp;
						ret->object.n->nobject.rational.den /= itmp;
						itmp = _igcd(ret->object.n->nobject.rational.num,ret->object.n->nobject.rational.den);
					}
				}
				if(ret->object.n->nobject.rational.den == 1)
				{
					itmp = ret->object.n->nobject.rational.num;
					ret->object.n->type = INTEGER;
					ret->object.n->nobject.z = itmp;
				}
				_read_return(ret);
			case TOK_COMPL:
				ret = (SExp *) hmalloc(sizeof(SExp));
				ret->type = NUMBER;
				ret->object.n = (Number *) hmalloc(sizeof(Number));
				ret->object.n->type = COMPLEX;
				if(buf[0] == '-' || buf[0] == '+')
					iter = 1;
				while(buf[iter] != '+' && buf[iter] != '-') iter++;
				ret->object.n->nobject.complex.r = atof(buf);
				ret->object.n->nobject.complex.i = atof(&buf[iter]);
				/* parse a complex, similar to a rational */
				_read_return(ret);
			case TOK_LPAREN:
				tmp1 = llread(fdin);
				if(tmp1 == fake_rpar)
					return snil;
				holder = cons(nil,snil);
				ret = holder;
				while(1)
				{
					//printf("tmp1->type == %s\n",typenames[tmp1->type]);
					if(tmp1 == seof || tmp1->type == SEOF)
					  {
					    if(feof(fdin))
						return makeerror(0,0,"received #e before end of list!");
					  }
					if(tmp1 == fake_rpar) 
					{
						//printf("tmp1 == fake_rpar\n");
						ret->object.clist.rest = snil;
						break;
					}
					else if(tmp1->type == ATOM && !strncmp(tmp1->object.str,".",1))
					{
						// dotted list
						ret->object.clist.rest = llread(fdin);
						tmp1 = llread(fdin);
						if(tmp1 != fake_rpar)
							return makeerror(0,0,"mal-formmated dotted list");
						break;
					}
					else
					{
						if(ret->object.clist.first == nil)
						{
							ret->object.clist.first = tmp1;
							//ret->object.clist.rest = cons(nil,snil);
						}
						else
						{
							ret->object.clist.rest = cons(tmp1,snil);
							ret = ret->object.clist.rest;
						}
					}
					tmp1 = llread(fdin);
				}
				_read_return(holder);
			case TOK_NQUOTE:
				tmp0 = llread(fdin);
				if(tmp0->type == ERROR)
					return tmp0;
				return cons(makeatom("quote"),cons(tmp0, snil));
			case TOK_MQUOTE:
				tmp0 = llread(fdin);
				if(tmp0->type == ERROR)
					return tmp0;
				return cons(makeatom("quasiquote"),cons(tmp0,snil));
			case TOK_UNQUOT:
				tmp0 = llread(fdin);
				if(tmp0->type == ERROR)
					return tmp0;
				return cons(makeatom("unquote"),cons(tmp0,snil));
			case TOK_SPLICE:
				tmp0 = llread(fdin);
				if(tmp0->type == ERROR)
					return tmp0;
				return cons(makeatom("unquote-splice"),cons(tmp0,snil));
			case TOK_RPAREN: 
				return fake_rpar;
			case TOK_LSQUAR: /* may be have to go fully recursive... */
				tmp1 = llread(fdin);
				if(tmp1 == fake_rsqr)
					return makevector(0,snil);
				holder = cons(nil,snil);
				ret = holder;
				while(1)
				{
					if(tmp1 == seof || tmp1->type == SEOF)
					  {
					    if(feof(fdin))
						return makeerror(0,0,"recieved #e before end of vector!");
					  }
					if(tmp1 == fake_rsqr) 
					{
						ret->object.clist.rest = snil;
						break;
					}
					else
					{
						/* NOTE this, and lparen above it,
						 * look like they are adding one extra
						 * cons to each list...
						 */
						if(ret->object.clist.first == nil)
						{
							ret->object.clist.first = tmp1;
							//ret->object.clist.rest = cons(nil,snil);
						}
						else
						{
							ret->object.clist.rest = cons(tmp1,snil);
							ret = ret->object.clist.rest;
						}
					}
					tmp1 = llread(fdin);
				}
				itmp = pairlength(holder);
				tmp0 = (SExp *)hmalloc(sizeof(SExp));
				tmp0->type = VECTOR;
				tmp0->object.vec = (SExp **)hmalloc(sizeof(SExp *) * itmp);
				tmp0->length = itmp;
				for(iter = 0;iter < itmp;iter++)
				{
					tmp0->object.vec[iter] = car(holder);
					holder = cdr(holder);
				}
				return tmp0;
			case TOK_RSQUAR:
				return fake_rsqr;
			case TOK_LCURLY:
				holder = makedict();
				while(1)
				{
					tmp0 = llread(fdin);
					if(tmp0 == seof)
						return makeerror(0,0,"#e received before end of dictionary literal!");
					if(tmp0 == fake_rcur)
						break;
					if(tmp0->type != STRING && tmp0->type != KEY && tmp0->type != ATOM)
						return makeerror(0,0,"invalid key type used in dict literal");
					tmp1 = llread(fdin);
					if(tmp1 == seof)
					  {
					    if(feof(fdin))
						return makeerror(0,0,"#e received before end of dictionary literal!");
					  }
					if(tmp1 == fake_rcur)
					{
						trie_put(tmp0->object.str,snil,holder->object.dict);
						break;
					}
					trie_put(tmp0->object.str,tmp1,holder->object.dict);
				}
				return holder;
			case TOK_RCURLY:
				return fake_rcur;
			case TOK_HERROR:
				return makeerror(0,0,"Syntax error: illegal hash object!");
				/* # error; return some S-Expression error type. */
			case TOK_SERROR:
				return makeerror(0,0,"reached #e before end of string");
			case TOK_LEOF:
				_read_return(seof);
			case TOK_LVOID:
				_read_return(svoid);
			case TOK_EOF:
				return seof;
			default:
				//printf("made it to default case: %d\n",tok);
				return snil;
		}
	}
	return snil;
}
char *
_itoa(char *b, int s, int *offset)
/* offset is a reach into b, so that
 * things like format & integer->string
 * can use _itoa on buffers that have already
 * been allocated...
 */
{
	int iter = *offset, rev_ptr = *offset, holder = 0;
	char c0 = ' ';
	if(b == nil)
		return nil;
	if(s < 0)
	{
		s *= -1;
		b[iter] = '-';
		iter++;
		rev_ptr++;
	}
	while(iter < MAX_STRING)
	{
		b[iter] = (s % 10) + '0';
		s /= 10;
		iter++;
		if(s == 0)
			break;
	}
	b[iter] = nul;
	holder = iter;
	for(iter--;rev_ptr < iter;rev_ptr++, iter--)
	{
		c0 = b[rev_ptr];
		b[rev_ptr] = b[iter];
		b[iter] = c0;
	}
	*offset = holder;
	return b;
}
char *
_itox(char *b, int s, int *offset)
{
	char *ret = b, tmp = 0;
	int iter = *offset, rev_iter = 0, holder = 0;
	if(b == nil)
		return nil;
	while(iter < MAX_STRING)
	{
		rev_iter = s & 0x0F;
		if(rev_iter >= 0 && rev_iter <= 9)
			b[iter] = rev_iter + '0';
		else if(rev_iter >= 0x0A && rev_iter <= 0x0F)
			b[iter] = (rev_iter - 0x0A) + 'A';
		s >>= 4;
		iter++;
		if(s == 0)
			break;	
	}
	b[iter] = nul;
	holder = iter;
	for(iter--, rev_iter = *offset; rev_iter <= iter;rev_iter++, iter--)
	{
		tmp = b[iter];
		b[iter] = b[rev_iter];
		b[rev_iter] = tmp;
	}
	*offset = holder;
	return b;
}
char *
_itoo(char *b, int s, int *offset)
{
	char *ret = b, tmp = 0;
	int iter = *offset, rev_iter = 0, holder = 0;
	if(b == nil)
		return nil;
	while(iter < MAX_STRING)
	{
		rev_iter = s & 07;
		if(rev_iter >= 0 && rev_iter <= 7)
			b[iter] = rev_iter + '0';
		s >>= 3;
		iter++;
		if(s == 0)
			break;	
	}
	b[iter] = nul;
	holder = iter;
	for(iter--, rev_iter = *offset; rev_iter <= iter;rev_iter++, iter--)
	{
		tmp = b[iter];
		b[iter] = b[rev_iter];
		b[rev_iter] = tmp;
	}
	*offset = holder;
	return b;
}
char *
_ftoa(char *b, double s, int *offset, int flag)
{
	char *tmp = nil;
	tmp = (char *)hmalloc(sizeof(char) * 64);
	if(flag)
		snprintf(tmp,64,"%E",s);
	else
		snprintf(tmp,64,"%g",s);
	return _strcpy(b,tmp,offset);
}
char *
_strcpy(char *dest, char *src, int *offset)
{
	int diter = *offset, siter = 0;
	if(dest == nil)
		return nil;
	while(src[siter] != nul)
		dest[diter++] = src[siter++];
	*offset = diter;
	return dest;
}
int
atox(char *a)
{
	int iter = 0, len = 0, ret = 0, sign = 1;
	if(a == nil)
		return 0;
	len = strlen(a);
	if(a[0] == '-')
	{
		sign = -1;
		iter++;
	}
	else if(a[0] == '+')
	{
		sign = 1;
		iter++;
	}
	for(;iter < len;iter++)
	{
		if(a[iter] >= 'a' && a[iter] <= 'f')
			ret += (a[iter] - 97 + 10);
		else if(a[iter] >= 'A' && a[iter] <= 'F')
			ret += (a[iter] - 55);
		else
			ret += (a[iter] - 48);
		if(iter < (len - 1))
			ret <<= 4;
	}
	return sign * ret;
}
int
atoo(char *a)
{
	int iter = 0, len = 0, ret = 0, sign = 1;
	if(a == nil)
		return 0;
	len = strlen(a);
	if(a[0] == '-')
	{
		sign = -1;
		iter++;
	}
	else if(a[0] == '+')
	{
		sign = 1;
		iter++;
	}
	for(;iter < len;iter++)
	{
		ret += (a[iter] - 48);
		if(iter < (len - 1))
			ret <<= 3;
	}
	return sign * ret;
}
int
atob(char *a)
{
	int iter = 0, len = 0, ret = 0, sign = 1;
	if(a == nil)
		return 0;
	len = strlen(a);
	if(a[0] == '-')
	{
		sign = -1;
		iter++;
	}
	else if(a[0] == '+')
	{
		sign = 1;
		iter++;
	}
	for(;iter < len;iter++)
	{
		ret <<= 1;
		ret += (a[iter] - 48);
	}
	return sign * ret;
}
int 
fmtlookup(char c)
{
	int rc = 0;
	char codes[] = {'A','a','s','S','x','X','o','O','~','f','F','i','%','R','m','M','C','c','l','L','b','B','g','G','n','N','v','V','t','T',0};
	while(codes[rc] != nul)
	{
		if(codes[rc] == c)
			return rc + 1;
		rc++;
	}
	return -1;
}
SExp *
format(SExp *rst, Symbol *e)
{
	int state = 0, bufiter = 0, iter = 0, cur_buf_size = 64, viter = 0;
	SExp *tmp = snil, *tmp1 = snil, *tmp2 = snil, *tmp3 = snil, *ret = snil, *src = snil;
	char *buf = nil, *str = nil;
	buf = (char *)hmalloc(sizeof(char) * cur_buf_size);
	src = car(rst);
	rst = cdr(rst);
	str = src->object.str;
__format_base:
	switch(state)
	{
		case 0: /* base case; basically, dispatch */
			if(iter >= src->length)
				break;
			if(bufiter >= (cur_buf_size - 2))
			{
				cur_buf_size += 64;
				buf = hrealloc(buf,sizeof(char) * cur_buf_size);
			}
			if(str[iter] == '~')
			{
				state = fmtlookup(str[iter + 1]);
				iter += 2;
			}
			else
			{
				buf[bufiter] = str[iter];
				bufiter++;
				iter++;
			}
			goto __format_base;
		case 1: /* A */
			// need to type dispatch here...
			tmp = car(rst);
			switch(tmp->type)
			{
				case KEY:
				case ATOM:
				case STRING:
					state = fmtlookup('S');
					goto __format_base;
				case BOOL:
					state = fmtlookup('B');
					goto __format_base;
				case GOAL:
					state = fmtlookup('G');
					goto __format_base;
				case VECTOR:
					state = fmtlookup('V');
					goto __format_base;
				case PAIR:
					state = fmtlookup('L');
					goto __format_base;
				case NUMBER:
					state = fmtlookup('N');
					goto __format_base;
				case CHAR:
					state = fmtlookup('C');
					goto __format_base;
				case NIL:
					rst = cdr(rst);
					buf[bufiter++] = '\'';
					buf[bufiter++] = '(';
					buf[bufiter++] = ')';
					state = 0;
					goto __format_base;
				case DICT:
					state = fmtlookup('T');
					goto __format_base;
				default: 
					break;
			}
			break;
		case 2: /* a */
			tmp = car(rst);
			switch(tmp->type)
			{
				case KEY:
				case ATOM:
				case STRING:
					state = fmtlookup('s');
					goto __format_base;
				case BOOL:
					state = fmtlookup('b');
					goto __format_base;
				case GOAL:
					state = fmtlookup('g');
					goto __format_base;
				case VECTOR:
					state = fmtlookup('v');
					goto __format_base;
				case PAIR:
					state = fmtlookup('l');
					goto __format_base;
				case NUMBER:
					state = fmtlookup('n');
					goto __format_base;
				case CHAR:
					state = fmtlookup('c');
					goto __format_base;
				case NIL:
					rst = cdr(rst);
					buf[bufiter++] = 'n';
					buf[bufiter++] = 'i';
					buf[bufiter++] = 'l';
					state = 0;
					goto __format_base;
				case DICT:
					state = fmtlookup('t');
					goto __format_base;
				default: 
					break;
			}
			break;
		case 3: /* s */
			tmp = car(rst);
			rst = cdr(rst);
			if((bufiter + tmp->length) >= cur_buf_size)
			{
				cur_buf_size += bufiter + tmp->length + 64;
				buf = hrealloc(buf,sizeof(char) * cur_buf_size);
			}
			if(tmp->type == STRING || tmp->type == ATOM || tmp->type == KEY)
				buf = _strcpy(buf,tmp->object.str,&bufiter);
			else
				buf[bufiter++] = '#';
			state = 0;
			goto __format_base;
		case 4: /* S */
			tmp = car(rst);
			rst = cdr(rst);
			if((bufiter + tmp->length + 2) >= cur_buf_size)
			{
				cur_buf_size += bufiter + tmp->length + 2 + 64;
				buf = hrealloc(buf,sizeof(char) * cur_buf_size);
			}
			if(tmp->type == STRING)
			{
				buf[bufiter++] = '"';
				buf = _strcpy(buf,tmp->object.str,&bufiter);
				buf[bufiter++] = '"';
			}
			else
				buf[bufiter++] = '#';
			state = 0;
			goto __format_base;
		case 5: /* x */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == INTEGER)
				buf = _itox(buf,AINT(tmp),&bufiter);
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 6: /* X */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == INTEGER)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = 'x';
				buf = _itox(buf,AINT(tmp),&bufiter);
			}
			else if(tmp->type == NUMBER && NTYPE(tmp) == REAL)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = 'x';
				buf = _itox(buf,(int)tmp3->object.n->nobject.real,&bufiter);
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 7: /* o */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == INTEGER)
				buf = _itoo(buf,AINT(tmp),&bufiter);
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 8: /* O */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == INTEGER)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = 'o';
				buf = _itoo(buf,AINT(tmp),&bufiter);
			}
			else if(tmp->type == NUMBER && NTYPE(tmp) == REAL)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = 'o';
				buf = _itoo(buf,(int)tmp3->object.n->nobject.real,&bufiter);
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 9: /* ~ */
			buf[bufiter++] = '~';
			state = 0;
			goto __format_base;
		case 10: /* f */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == REAL)
				buf = _ftoa(buf,AREAL(tmp),&bufiter,0);
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 11: /* F */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == REAL)
				buf = _ftoa(buf,AREAL(tmp),&bufiter,1);
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 12: /* i */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == NUMBER && NTYPE(tmp) == INTEGER)
				buf = _itoa(buf,AINT(tmp),&bufiter);
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
			}
			state = 0;
			goto __format_base;
		case 13: /* % */
			buf[bufiter++] = '\n';
			state = 0;
			goto __format_base;
		case 14: /* R */
		case 15: /* m */
		case 16: /* M */
		case 17: /* C */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != CHAR)
				buf[bufiter++] = '#';
			else
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = '\\';
				buf[bufiter++] = tmp->object.c;
			}
			state = 0;
			goto __format_base;
		case 18: /* c */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != CHAR)
				buf[bufiter++] = '#';
			else
				buf[bufiter++] = tmp->object.c;
			state = 0;
			goto __format_base;
		case 19: /* l */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != PAIR)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			while(tmp != snil)
			{
				tmp1 = car(tmp);
				tmp = cdr(tmp);
				tmp2 = format(cons(makestring("~a"),cons(tmp1, snil)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				buf[bufiter++] = ' ';
			}
			state = 0;
			goto __format_base;
		case 20: /* L */
			/* (format "~L" (list (list "test0" "test1") "test2"))
			 * ("test0""test2")
			 */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != PAIR)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			buf[bufiter++] = '(';
			while(tmp != snil)
			{
				tmp1 = car(tmp);
				tmp = cdr(tmp);
				tmp2 = format(cons(makestring("~A"),cons(tmp1, snil)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				buf[bufiter++] = ' ';
			}
			buf[bufiter++] = ')';
			state = 0;
			goto __format_base;
		case 21: /* b */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == BOOL)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = tmp->object.c ? 't' : 'f';
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'B';
			}
			state = 0;
			goto __format_base;
		case 22: /* B */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == BOOL)
			{
				if(tmp->object.c)
					buf = _strcpy(buf,"true",&bufiter);
				else
					buf = _strcpy(buf,"false",&bufiter);
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'B';
			}
			state = 0;
			goto __format_base;
		case 23: /* g */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == GOAL)
			{
				buf[bufiter++] = '#';
				buf[bufiter++] = tmp->object.c ? 's' : 'u';
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'G';
			}
			state = 0;
			goto __format_base;
		case 24: /* G */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type == BOOL)
			{
				if(tmp->object.c)
					buf = _strcpy(buf,"successful",&bufiter);
				else
					buf = _strcpy(buf,"unsuccessful",&bufiter);
			}
			else
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'G';
			}
			state = 0;
			goto __format_base;
		case 25: /* n */
		case 26: /* N */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != NUMBER)
			{
				buf[bufiter++] = 'N';
				buf[bufiter++] = 'a';
				buf[bufiter++] = 'N';
				state = 0;
				goto __format_base;
			}
			switch(NTYPE(tmp))
			{
				case INTEGER:
					buf = _itoa(buf,AINT(tmp),&bufiter);
					break;
				case REAL:
					buf = _ftoa(buf,AREAL(tmp),&bufiter,0);
					break;
				case RATIONAL:
					buf = _itoa(buf,NUM(tmp),&bufiter);
					buf[bufiter++] = '/';
					buf = _itoa(buf,DEN(tmp),&bufiter);
					break;
				case COMPLEX:
					buf = _ftoa(buf,CEREAL(tmp),&bufiter,0);
					if(IMMAG(tmp) >= 0.0)
						buf[bufiter++] = '+';
					buf = _ftoa(buf,IMMAG(tmp),&bufiter,0);
					buf[bufiter++] = 'i';
					break;
			}
			state = 0;
			goto __format_base;
		case 27: /* v */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != VECTOR)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			for(viter = 0; viter < tmp->length; viter++)
			{
				tmp1 = tmp->object.vec[viter];
				tmp2 = format(cons(makestring("~a"),cons(tmp1, snil)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				if(viter < (tmp->length - 1))
					buf[bufiter++] = ' ';
			}
			state = 0;
			goto __format_base;
		case 28: /* V */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != VECTOR)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			buf[bufiter++] = '[';
			for(viter = 0; viter < tmp->length; viter++)
			{
				tmp1 = tmp->object.vec[viter];
				tmp2 = format(cons(makestring("~A"),cons(tmp1, snil)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64; // way too much
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				if(viter < (tmp->length - 1))
					buf[bufiter++] = ' ';
			}
			buf[bufiter++] = ']';
			state = 0;
			goto __format_base;
		case 29: /* t */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != DICT)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			tmp1 = mcar(trie_keys(tmp->object.dict,tconcify(snil)));
			while(tmp1 != e->snil)
			{
				tmp2 = car(tmp1);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				buf[bufiter++] = ':';
				buf[bufiter++] = ' ';
				tmp2 = format(list(2,makestring("~a"),trie_get(tmp2->object.str,tmp->object.dict)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				tmp1 = cdr(tmp1);
				if(tmp1 != snil)
					buf[bufiter++] = ',';
			}	
			state = 0;
			goto __format_base;
		case 30: /* T */
			tmp = car(rst);
			rst = cdr(rst);
			if(tmp->type != DICT)
			{
				buf[bufiter++] = '#';
				state = 0;
				goto __format_base;
			}
			tmp1 = mcar(trie_keys(tmp->object.dict,tconcify(snil)));
			buf[bufiter++] = '{';
			while(tmp1 != e->snil)
			{
				tmp2 = car(tmp1);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf[bufiter++] = '"';
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				buf[bufiter++] = '"';
				buf[bufiter++] = ' ';
				tmp2 = format(list(2,makestring("~A"),trie_get(tmp2->object.str,tmp->object.dict)),e);
				if((bufiter + tmp2->length) >= cur_buf_size)
				{
					cur_buf_size += bufiter + tmp2->length + 64;
					buf = hrealloc(buf,sizeof(char) * cur_buf_size);
				}
				buf = _strcpy(buf,tmp2->object.str,&bufiter);
				tmp1 = cdr(tmp1);
				if(tmp1 != snil)
					buf[bufiter++] = ' ';
			}	
			buf[bufiter++] = '}';
			state = 0;
			goto __format_base;
		case 999: /* return case */
			break;
	}
	buf[bufiter] = nul;
	ret = (SExp *)hmalloc(sizeof(SExp));
	ret->type = STRING;
	ret->object.str = buf;
	ret->length = bufiter;
	return ret;
}
SExp *
fcoerce(SExp *from, SExp *to)
{
	SExp *ret = nil, *tmp = nil;
	int iter = 0;
	if(to->type != KEY && to->type != ATOM && to->type != STRING)
		return makeerror(1,0,"coerce: the to parameter *must* be a (KEYWORD | ATOM | STRING)");
	switch(from->type)
	{
		case NUMBER:
			switch(NTYPE(from))
			{
				case INTEGER:
					if(!strncasecmp(to->object.str,"char",4))
						return makechar(AINT(from) & 0xFF);
					return makeerror(1,0,"coerce: illegal coercion.");
				default:
					return makeerror(1,0,"coerce: unknown coercion attempt");
			}
		case CHAR:
			if(!strncasecmp(to->object.str,"int",3))
			{
				iter = from->object.c;
				iter &= 255;
				return makeinteger(iter);
			}
			else
				return makeerror(1,0,"coerce: unknown coercion attempt");
		case KEY:
			if(!strncasecmp(to->object.str,"string",6))
				return makestring(from->object.str);
			else if(!strncasecmp(to->object.str,"atom",4) || !strncasecmp(to->object.str,"symbol",6))
				return makeatom(from->object.str);
			else if(!strncasecmp(to->object.str,"vector",6))
			{
				ret = makevector(from->length,nil);
				for(;iter < from->length;iter++)
					ret->object.vec[iter] = makechar(from->object.str[iter]);
				return ret;
			}
			else
				return makeerror(2,0,"coerce: unknown coercion attempt");
		case ATOM:
			if(!strncasecmp(to->object.str,"string",6))
				return makestring(from->object.str);
			else if(!strncasecmp(to->object.str,"key",3))
			{
				ret = makeatom(from->object.str);
				ret->type = KEY;
				return ret;
			}
			else if(!strncasecmp(to->object.str,"vector",6))
			{
				ret = makevector(from->length,nil);
				for(;iter < from->length;iter++)
					ret->object.vec[iter] = makechar(from->object.str[iter]);
				return ret;
			}
			else
				return makeerror(2,0,"coerce: unknown coercion attempt");
		case VECTOR:
			if(!strncasecmp(to->object.str,"string",6))
			{
				ret = makestring_v(from->length,nul);
				for(;iter < from->length;iter++)
				{
					if(from->object.vec[iter]->type != CHAR)
						return makeerror(2,0,"coerce: illegal coercion attempt: vector containing non-chars to string");
					ret->object.str[iter] = from->object.vec[iter]->object.c;
				}
				return ret;
			}
			else if(!strncasecmp(to->object.str,"pair",4))
			{
				ret = snil;
				for(iter = from->length - 1;iter >= 0;iter--)
					ret = cons(from->object.vec[iter],ret);
				return ret;
			}
			else
				return makeerror(2,0,"coerce: unknown coercion attempt");
		case PAIR:
			if(!strncasecmp(to->object.str,"vector",6))
			{
				ret = makevector(pairlength(from),nil);
				for(;iter < ret->length;iter++)
				{
					ret->object.vec[iter] = car(from);
					from = cdr(from);
				}
				return ret;
			}
			else
				return makeerror(2,0,"coerce: unknown coercion attempt");
		case STRING:
			if(!strncasecmp(to->object.str,"key",3))
			{
				ret = makeatom(from->object.str);
				ret->type = KEY;
				return ret;
			}
			else if(!strncasecmp(to->object.str,"atom",4))
				return makeatom(from->object.str);
			else if(!strncasecmp(to->object.str,"vector",6))
			{
				ret = makevector(from->length,nil);
				for(;iter < from->length;iter++)
					ret->object.vec[iter] = makechar(from->object.str[iter]);
				return ret;
			}
			else if(!strncasecmp(to->object.str,"pair",4))
			{
				ret = cons(snil,snil);
				tmp = ret;
				for(;iter < from->length;iter++)
				{
					mcar(tmp) = makechar(from->object.str[iter]);
					mcdr(tmp) = cons(snil,snil);
					tmp = mcdr(tmp);
				}
				return ret;
			}
			else if(!strncasecmp(to->object.str,"int",3))
				return makeinteger(strtol(from->object.str,nil,10));
			else if(!strncasecmp(to->object.str,"real",4))
				return makereal(strtod(from->object.str,nil));
			else
				return makeerror(1,0,"coerce: unknown coercion attempt");
		default:
			return makeerror(1,0,"coerce: unknown coercion attempt");
	}
	return makeerror(1,0,"coerce: unknown coercion attempt");
}
SExp *
__seval(SExp *s, Symbol *e)
{
	SExp *src = s, *fst = snil, *rst = snil, *tmp0 = snil, *tmp1 = snil, *tmp2 = snil, *stk = snil, *__r = snil, *__val = snil, *ret = snil;
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
	e->tick++;
	switch(state)
	{
		case __PRE_APPLY:
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
						else if(tmp1->type == KEY)
						{
							tmp2 = car(cdr(tmp0));
							if(!strncasecmp(tmp1->object.str,"opt",3))
							{
								if(tmp2->type == PAIR)
								{
									tmp1 = car(tmp2);
									tmp2 = car(cdr(tmp2));
									if(rst != snil)
										add_env(env,tmp1->object.str,car(rst));
									else
									{
										// these should be evaluted at bind time, not now. Fix this
										if(tmp2->type == PAIR) // *should* do the jump/shuck, but for now, laze out...
										{
											printf("\tMade it to the \"should have done the jump/shuck\" call?\n");
											tmp2 = __seval(tmp2,env);
											//stk = cons(list(5,src,fst,cdr(rst),ret,tenv),stk);
										}
										else if(tmp2->type == ATOM)
										{
											tmp2 = symlookup(tmp2->object.str,env);
											if(tmp2 == nil)
											{
												__return(makeerror(1,0,"Unknown atom in default position"));
											}
										}
										add_env(env,tmp1->object.str,tmp2);
									}
								}	
								else if(tmp2->type == ATOM)
									add_env(env,tmp1->object.str,car(rst));
							}
							else if(!strncasecmp(tmp1->object.str,"rest",4))
							{
								tmp1 = car(cdr(tmp0));
								add_env(env,tmp1->object.str,rst);
								rst = snil;
							}
							else if(!strncasecmp(tmp1->object.str,"body",4))
							{
								tmp1 = car(cdr(tmp0));
								if(rst == snil)
								{
									__return(makeerror(1,0,":body specified, but argument list was nil"));
								}
								add_env(env,tmp1->object.str,rst);
								rst = snil;
							}
							tmp0 = cdr(tmp0);
						}
						tmp0 = cdr(tmp0);
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
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"def sym : SYMBOL s : SEXPRESSION => #v"));
			}
			__return(fdef(car(rst),car(cdr(rst)),env)); // do we really need to __return? I guess if you want the #v...
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
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"eval f : FORM => S-EXPRESSION"));
			}
			__return(__seval(car(rst),env));
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
			switch(pairlength(rst))
			{
				case 0:
					__return(snil);
				case 1:
					__return(car(rst));
				case 2:
					__return(bappend(car(rst),car(cdr(rst))));
				default:
					__return(append(rst));
			}
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
			if(pairlength(rst) != 1)
			{
				__return(makeerror(1,0,"help t : OBJECT => STRING "));
			}
			__return(fmeta(car(rst)));
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
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"def sym : SYMBOL s : SEXPRESSION => #v"));
			}
			__return(fdef(car(rst),car(cdr(rst)),(Symbol *)car(cdr(cdr(rst)))->object.foreign));
		case OPSETENV:
			if(pairlength(rst) != 2)
			{
				__return(makeerror(1,0,"set! expects exactly two arguments..."));
			}
			__return(fset(car(rst),car(cdr(rst)),(Symbol *)car(cdr(cdr(rst)))->object.foreign));
		case OPDEFAULTENV:
		case OPNULLENV:
		case OPSTDENV:
			__return(svoid);
		case __INTERNAL_RETURN:
			/* basically, this is the old __retstate;
			 * Flattening everything should help with total speed...
			 */
			__r = car(stk);
			stk = cdr(stk);
			if(__val->type == ERROR)
				return __val;
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
				return makeerror(1,0,"no arguments given to function with at least one non-optional parameter");
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

/* math functions */
SExp *
fsqrt(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"sqrt operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = sqrt(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = sqrt(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = sqrt((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fden(SExp *tmp0)
{
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && tmp0->object.n->type != RATIONAL))
		return makeerror(1,0,"type clash: denomenator expects a rational...");
	return makeinteger(tmp0->object.n->nobject.rational.den);
}
SExp *
fnum(SExp *tmp0)
{
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && tmp0->object.n->type != RATIONAL))
		return makeerror(1,0,"type clash: numerator expects a rational...");
	return makeinteger(tmp0->object.n->nobject.rational.num);
}
SExp *
fplus(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = 0;
	tmp1 = makenumber(INTEGER);
	tmp1->object.n->nobject.z = pairlength(rst);
	if(tmp1->object.n->nobject.z == 0)
		return tmp1;
	if(tmp1->object.n->nobject.z == 1)
	{
		if(mcar(rst)->type == NUMBER)
			return car(rst);
		else
			return makeerror(1,0,"type clash: + works only on numbers...");
	}
	else
	{
		tmp0 = makenumber(INTEGER);
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"type clash: + only operates on numbers...");
		while(rst != snil)
		{
			switch(tmp1->object.n->type)
			{
				case INTEGER:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							tmp0->object.n->nobject.z += tmp1->object.n->nobject.z;
							break;
						case REAL:
							tmp0->object.n->nobject.real += tmp1->object.n->nobject.z;
							break;
						case RATIONAL:
    						tmp0->object.n->nobject.rational.num += (tmp1->object.n->nobject.z * tmp0->object.n->nobject.rational.den); 
							break;
						case COMPLEX: /* nothing for now */
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.z;
							break;
					}
					break;
				case REAL:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							/*
							tmp1->object.n->nobject.real += tmp0->object.n->nobject.z;
							tmp0 = tmp1; */
							tmp0->object.n->nobject.real = tmp0->object.n->nobject.z * 1.0;
							tmp0->object.n->nobject.real += tmp1->object.n->nobject.real;
							tmp0->object.n->type = REAL;
							break;
						case REAL:
							tmp0->object.n->nobject.real += tmp1->object.n->nobject.real;
							break;
						case RATIONAL:
							/*
							printf("[!] I *should* be adding a rational & a real (tmp0 & tmp1 resp.): ");
							princ(tmp0);
							printf(" , ");
							princ(tmp1);
							printf("\n");
							printf("%f\n",(tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0));
							*/
							tmp0->object.n->nobject.real = (tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0);
							tmp0->object.n->type = REAL;
							/*
							princ(tmp0);
							printf("\n");
							*/
							tmp0->object.n->nobject.real += tmp1->object.n->nobject.real;
							/*
							princ(tmp0);
							printf("\n");
							*/
							break;
						case COMPLEX:
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.real;
					}
					break;
				case COMPLEX:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							/*
							tmp1->object.n->nobject.real += tmp0->object.n->nobject.z;
							tmp0 = tmp1; */
							tmp0->object.n->nobject.complex.r = tmp0->object.n->nobject.z * 1.0;
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.complex.r;
							tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
							tmp0->object.n->type = COMPLEX;
							break;
						case REAL:
							//tmp0->object.n->nobject.real += tmp1->object.n->nobject.real;
							tmp0->object.n->nobject.complex.r = tmp0->object.n->nobject.real;
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.complex.r;
							tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
							tmp0->object.n->type = COMPLEX;
							break;
						case RATIONAL:
							tmp0->object.n->nobject.complex.r = (tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0);
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.complex.r;
							tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
							tmp0->object.n->type = COMPLEX;
							break;
						case COMPLEX:
							tmp0->object.n->nobject.complex.r += tmp1->object.n->nobject.complex.r;
							tmp0->object.n->nobject.complex.i += tmp1->object.n->nobject.complex.i;
					}
					break;
				case RATIONAL:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z * tmp1->object.n->nobject.rational.den;
							tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
							tmp0->object.n->nobject.rational.num += tmp1->object.n->nobject.rational.num;
							tmp0->object.n->type = RATIONAL;
							break;
						case REAL:
							tmp0->object.n->nobject.real += (tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0);
							break;
						case RATIONAL:
							tmp0->object.n->nobject.rational.num *= tmp1->object.n->nobject.rational.den;
							tmp0->object.n->nobject.rational.num += (tmp1->object.n->nobject.rational.num * tmp0->object.n->nobject.rational.den);
							tmp0->object.n->nobject.rational.den *= tmp1->object.n->nobject.rational.den;
							itmp = _igcd(tmp0->object.n->nobject.rational.num,tmp0->object.n->nobject.rational.den);
							if(itmp != 1)
							{
								tmp0->object.n->nobject.rational.num /= itmp;
								tmp0->object.n->nobject.rational.den /= itmp;
							}
							break;
						case COMPLEX:
							tmp0->object.n->nobject.complex.r += (tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0);
							break;
					}
					break;
			}
			rst = cdr(rst);
			tmp1 = car(rst);
		}
	}
	return tmp0;
}
SExp *
fmult(SExp *rst)
{
	int itmp = 0;
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	double ftmp = 0.0;
	itmp = pairlength(rst);
	if(itmp == 0)
		return makenumber(INTEGER);
	if(itmp == 1)
	{
		if(mcar(rst)->type == NUMBER)
			return car(rst);
		else
			return makeerror(1,0,"type clash: * works only on numbers...");
	}
	else
	{
		tmp0 = makenumber(INTEGER);
		tmp0->object.n->nobject.z = 1;
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"type clash: + only operates on numbers...");
		//printf("[!] at loop level for #\\*\n");
		while(rst != snil)
		{
			switch(tmp1->object.n->type)
			{
				case INTEGER:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							//printf("\t[!] Integer * Integer?\n");
							tmp0->object.n->nobject.z *= tmp1->object.n->nobject.z;
							break;
						case REAL:
							//printf("\t[!] Integer * Real?\n");
							tmp0->object.n->nobject.real *= tmp1->object.n->nobject.z;
							break;
						case RATIONAL:
	   									tmp0->object.n->nobject.rational.num *= tmp1->object.n->nobject.z; 
							break;
						case COMPLEX: /* nothing for now */
							CEREAL(tmp0) = CEREAL(tmp0) * tmp1->object.n->nobject.z;
							IMMAG(tmp0) = IMMAG(tmp0) * tmp1->object.n->nobject.z;
							break;
					}
					break;
				case REAL:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							tmp0->object.n->nobject.real = tmp0->object.n->nobject.z * 1.0;
							tmp0->object.n->nobject.real *= tmp1->object.n->nobject.real;
							tmp0->object.n->type = REAL;
							break;
						case REAL:
							tmp0->object.n->nobject.real *= tmp1->object.n->nobject.real;
							break;
						case RATIONAL:
							tmp0->object.n->nobject.real = (tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0);
							tmp0->object.n->type = REAL;
							tmp0->object.n->nobject.real *= tmp1->object.n->nobject.real;
							break;
						case COMPLEX:
							CEREAL(tmp0) = tmp1->object.n->nobject.real * CEREAL(tmp0);
							IMMAG(tmp0) = tmp1->object.n->nobject.real * IMMAG(tmp0);
							break;
					}
					break;
				case RATIONAL:
					switch(tmp0->object.n->type)
					{
						case INTEGER:
							tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z * tmp1->object.n->nobject.rational.num;
							tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
							tmp0->object.n->type = RATIONAL;
							break;
						case REAL:
							tmp0->object.n->nobject.real *= (tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0);
							break;
						case RATIONAL:
							tmp0->object.n->nobject.rational.num *= tmp1->object.n->nobject.rational.num;
							tmp0->object.n->nobject.rational.den *= tmp1->object.n->nobject.rational.den;
							itmp = _igcd(tmp0->object.n->nobject.rational.num,tmp0->object.n->nobject.rational.den);
							if(itmp != 1)
							{
								tmp0->object.n->nobject.rational.num /= itmp;
								tmp0->object.n->nobject.rational.den /= itmp;
							}
							break;
						case COMPLEX:
							ftmp = ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
							CEREAL(tmp0) = ftmp * CEREAL(tmp0);
							IMMAG(tmp0) = ftmp * IMMAG(tmp0);
							break;
					}
					break;
				case COMPLEX:
					switch(NTYPE(tmp0))
					{
						case INTEGER:
							/*printf("made it to integer * complex\n");
							printf("%s * %s\n", numtypes[NTYPE(tmp0)], numtypes[NTYPE(tmp1)]);
							printf("tmp0 => ");
							princ(tmp0);
							printf(", tmp1 => ");
							princ(tmp1);
							printf("\n");
							printf("#C(%f %f)\n",CEREAL(tmp1),IMMAG(tmp1));
							printf("#C(%f %f)\n",tmp0->object.n->nobject.z * CEREAL(tmp1), tmp0->object.n->nobject.z * IMMAG(tmp1));*/
							ftmp = tmp0->object.n->nobject.z * IMMAG(tmp1);
							/* upgrade to complex; (int + 0i) * (c + d) -> [int * c] + [int * d] */ 
							CEREAL(tmp0) = tmp0->object.n->nobject.z * CEREAL(tmp1);
							IMMAG(tmp0) = ftmp;
							NTYPE(tmp0) = COMPLEX;
							break;
						case RATIONAL:
							ftmp = ((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0));
							CEREAL(tmp0) = ftmp * CEREAL(tmp1);
							IMMAG(tmp0) = ftmp * IMMAG(tmp1);
							NTYPE(tmp0) = COMPLEX;
							break;
						case REAL:
							/*printf("%s * %s\n", numtypes[NTYPE(tmp0)], numtypes[NTYPE(tmp1)]);
							printf("tmp0 => ");
							princ(tmp0);
							printf(", tmp1 => ");
							princ(tmp1);
							printf("\n");
							printf("#C(%f %f)\n",CEREAL(tmp1),IMMAG(tmp1));
							printf("#C(%f %f)\n",tmp0->object.n->nobject.real * CEREAL(tmp1), tmp0->object.n->nobject.real * IMMAG(tmp1));*/
							ftmp = tmp0->object.n->nobject.real;
							CEREAL(tmp0) = ftmp * CEREAL(tmp1);
							IMMAG(tmp0) = ftmp * IMMAG(tmp1);
							NTYPE(tmp0) = COMPLEX;
							break;
						case COMPLEX:
							/*printf("%s * %s\n", numtypes[NTYPE(tmp0)], numtypes[NTYPE(tmp1)]);
							printf("tmp0 => ");
							princ(tmp0);
							printf(", tmp1 => ");
							princ(tmp1);
							printf("\n");
							printf("#C(%f %f)\n",CEREAL(tmp1),IMMAG(tmp1));*/
							ftmp = (IMMAG(tmp0) * CEREAL(tmp1)) + (CEREAL(tmp0) * IMMAG(tmp1));
							CEREAL(tmp0) = (CEREAL(tmp0) * CEREAL(tmp1)) - (IMMAG(tmp0) * IMMAG(tmp1));
							IMMAG(tmp0) = ftmp;
							break;
					}
			}
			rst = cdr(rst);
			tmp1 = car(rst);
		}
	}
	return tmp0;
}
SExp *
fsubt(SExp *rst)
{
	int itmp = 0;
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	itmp = pairlength(rst);
	if(itmp == 0)
		return makeerror(1,0,"- expects at least one argument...");
	tmp0 = makenumber(INTEGER);
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"- operates only on numbers...");
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			if(itmp == 1)
				tmp0->object.n->nobject.z = - tmp1->object.n->nobject.z;
			else
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->type = REAL;
			if(itmp == 1)
				tmp0->object.n->nobject.real = -tmp1->object.n->nobject.real;
			else
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			break;
		case RATIONAL:
			tmp0->object.n->type = RATIONAL;
			if(itmp == 1)
				tmp0->object.n->nobject.rational.num = -tmp1->object.n->nobject.rational.num;
			else
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			break;
		case COMPLEX:
			NTYPE(tmp0) = COMPLEX;
			if(itmp == 1)
			{
				CEREAL(tmp0) = -CEREAL(tmp1);
				IMMAG(tmp0) = -IMMAG(tmp1);
			}
			else
			{
				CEREAL(tmp0) = CEREAL(tmp1);
				IMMAG(tmp0) = IMMAG(tmp1);
			}
			break;
	}
	if(itmp == 1)
		return tmp0;
	tmp1 = car(rst);
	while(rst != snil)
	{
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.z -= tmp1->object.n->nobject.z;
						break;
					case REAL:
						tmp0->object.n->nobject.real -= tmp1->object.n->nobject.z;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num -= (tmp1->object.n->nobject.z * tmp0->object.n->nobject.rational.den); 
						break;
					case COMPLEX: /* nothing for now */
						CEREAL(tmp0) -= tmp1->object.n->nobject.z;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						/*
						tmp1->object.n->nobject.real += tmp0->object.n->nobject.z;
						tmp0 = tmp1; */
						tmp0->object.n->nobject.real = tmp0->object.n->nobject.z * 1.0;
						tmp0->object.n->nobject.real -= tmp1->object.n->nobject.real;
						tmp0->object.n->type = REAL;
						break;
					case REAL:
						tmp0->object.n->nobject.real -= tmp1->object.n->nobject.real;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.real = (tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0);
						tmp0->object.n->type = REAL;
						tmp0->object.n->nobject.real -= tmp1->object.n->nobject.real;
						break;
					case COMPLEX:
						CEREAL(tmp0) -= tmp1->object.n->nobject.real;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z * tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.num -= tmp1->object.n->nobject.rational.num;
						tmp0->object.n->type = RATIONAL;
						break;
					case REAL:
						tmp0->object.n->nobject.real -= (tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0);
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num *= tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.num -= (tmp1->object.n->nobject.rational.num * tmp0->object.n->nobject.rational.den);
						tmp0->object.n->nobject.rational.den *= tmp1->object.n->nobject.rational.den;
						itmp = _igcd(tmp0->object.n->nobject.rational.num,tmp0->object.n->nobject.rational.den);
						if(itmp != 1)
						{
							tmp0->object.n->nobject.rational.num /= itmp;
							tmp0->object.n->nobject.rational.den /= itmp;
						}
						break;
					case COMPLEX:
						CEREAL(tmp0) -= (tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0);
				}
				break;
			case COMPLEX:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						CEREAL(tmp0) = tmp0->object.n->nobject.z - CEREAL(tmp1);
						IMMAG(tmp0) = 0.0 - IMMAG(tmp1);
						NTYPE(tmp0) = COMPLEX;
						break;
					case RATIONAL:
						CEREAL(tmp0) = ((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) - CEREAL(tmp1);
						IMMAG(tmp0) = 0.0 - IMMAG(tmp1);
						NTYPE(tmp0) = COMPLEX;
						break;
					case REAL:
						CEREAL(tmp0) = tmp0->object.n->nobject.real - CEREAL(tmp1);
						IMMAG(tmp0) = 0.0 - IMMAG(tmp1);
						NTYPE(tmp0) = COMPLEX;
						break;
					case COMPLEX:
						CEREAL(tmp0) -= CEREAL(tmp1);
						IMMAG(tmp0) -= IMMAG(tmp1);
						break;
				}
				break;
		}
		rst = cdr(rst);
		tmp1 = car(rst);
	}
	return tmp0;
}
SExp *
fdivd(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	int itmp = 0;
	double ftmp = 0.0, f0 = 0.0, f1 = 0.0, f2 = 0.0;
	tmp0 = makenumber(INTEGER);
	itmp = pairlength(rst);
	if(itmp == 0)
		return makeerror(1,0,"/ needs at least one argument...");
	else if(itmp == 1)
	{
		if(NTYPE(mcar(rst)) == COMPLEX)
		{
			tmp1 = car(rst);
			ftmp = (CEREAL(tmp1) * CEREAL(tmp1)) + (IMMAG(tmp1) * IMMAG(tmp1));
			CEREAL(tmp0) = CEREAL(tmp1) / ftmp;
			IMMAG(tmp0) = ( - IMMAG(tmp1)) / ftmp;
			NTYPE(tmp0) = COMPLEX;
			return tmp0;
		}
		else
			tmp0->object.n->nobject.z = 1;
	}	
	else
	{
		tmp1 = car(rst);
		rst = cdr(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"/ operates on numbers only...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
				break;
			case REAL:
				tmp0->object.n->type = REAL;
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
				break;
			case RATIONAL:
				tmp0->object.n->type = RATIONAL;
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
				tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
				break;
			case COMPLEX:
				CEREAL(tmp0) = CEREAL(tmp1);
				IMMAG(tmp0) = IMMAG(tmp1);
				NTYPE(tmp0) = COMPLEX;
				break;
		}
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"/ operates on numbers only...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				if(tmp1->object.n->nobject.z == 0)
					return makeerror(1,0,"division by zero (& infinity isnt currently a number)");
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z % tmp1->object.n->nobject.z) == 0)
							tmp0->object.n->nobject.z /= tmp1->object.n->nobject.z;
						else
						{
							tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z;
							tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.z;
							tmp0->object.n->type = RATIONAL;
						}
						break;
					case REAL:
						tmp0->object.n->nobject.real /= tmp1->object.n->nobject.z * 1.0;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.den *= tmp1->object.n->nobject.z;
						break;
					case COMPLEX:
						ftmp = (tmp1->object.n->nobject.z * 1.0) * (tmp1->object.n->nobject.z * 1.0);
						CEREAL(tmp0) = (CEREAL(tmp0) * tmp1->object.n->nobject.z) / ftmp;
						IMMAG(tmp0) = (IMMAG(tmp0) * tmp1->object.n->nobject.z) / ftmp;
						break;
				}
				break;
			case REAL:
				if(tmp1->object.n->nobject.real == 0.0)
					return makeerror(1,0,"division by zero (& inexact infinity is not currently a number...)");
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.real = (tmp0->object.n->nobject.z * 1.0) / tmp1->object.n->nobject.real;
						tmp0->object.n->type = REAL;
						break;
					case REAL:
						tmp0->object.n->nobject.real /= tmp1->object.n->nobject.real;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.real = ((tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0)) / tmp1->object.n->nobject.real;
						tmp0->object.n->type = REAL;
						break;
					case COMPLEX:
						ftmp = tmp1->object.n->nobject.real * tmp1->object.n->nobject.real;
						CEREAL(tmp0) = (CEREAL(tmp0) * tmp1->object.n->nobject.real) / ftmp;
						IMMAG(tmp0) = (IMMAG(tmp0) * tmp1->object.n->nobject.real) / ftmp;
						break;
				}
				break;
			case RATIONAL:
				if(tmp1->object.n->nobject.rational.num == 0 || tmp1->object.n->nobject.rational.den == 0)
					return makeerror(1,0,"division by zero (& exact infinity isn't currently a number...)");
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z * tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.num;
						tmp0->object.n->type = RATIONAL;
						break;
					case REAL:
						tmp0->object.n->nobject.real = ((tmp1->object.n->nobject.rational.num * 1.0) / (tmp1->object.n->nobject.rational.den * 1.0)) / tmp0->object.n->nobject.real;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num *= tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.den *= tmp1->object.n->nobject.rational.num;
						break;
					case COMPLEX:
						ftmp = (NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0);
						ftmp *= ftmp;
						CEREAL(tmp0) = (CEREAL(tmp0) * (NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)) / ftmp;
						IMMAG(tmp0) = (IMMAG(tmp0) * (NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)) / ftmp;
						break;
				}
			case COMPLEX:
				switch(NTYPE(tmp0))
				{
					case INTEGER:
						ftmp = (CEREAL(tmp1) * CEREAL(tmp1)) + (IMMAG(tmp1) * IMMAG(tmp1));
						f0 = tmp0->object.n->nobject.z * 1.0;
						CEREAL(tmp0) = (f0 * CEREAL(tmp1)) / ftmp;
						IMMAG(tmp0) = (f0 * IMMAG(tmp1)) / ftmp;
						NTYPE(tmp0) = COMPLEX;
						break;
					case REAL:
						ftmp = (CEREAL(tmp1) * CEREAL(tmp1)) + (IMMAG(tmp1) * IMMAG(tmp1));
						f0 = tmp0->object.n->nobject.real;
						CEREAL(tmp0) = (f0 * CEREAL(tmp1)) / ftmp;
						IMMAG(tmp0) = (f0 * IMMAG(tmp1)) / ftmp;
						NTYPE(tmp0) = COMPLEX;
						break;
					case RATIONAL:
						ftmp = (CEREAL(tmp1) * CEREAL(tmp1)) + (IMMAG(tmp1) * IMMAG(tmp1));
						f0 = (NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0);
						CEREAL(tmp0) = (f0 * CEREAL(tmp1)) / ftmp;
						IMMAG(tmp0) = (f0 * IMMAG(tmp1)) / ftmp;
						NTYPE(tmp0) = COMPLEX;
						break;
					case COMPLEX:
						/* (a + b) ÷ (c + d) -> [(ac + bd) ÷ (c^2 + d^2)] + [(bc - ad) ÷ (c ^ 2 + d ^ 2)] */
						f0 = (CEREAL(tmp1) * CEREAL(tmp1)) + (IMMAG(tmp1) * IMMAG(tmp1));
						ftmp = ((CEREAL(tmp0) * CEREAL(tmp1)) + (IMMAG(tmp0) * IMMAG(tmp1))) / f0;
						f1 = CEREAL(tmp0);
						f2 = IMMAG(tmp0);
						/*printf("%f %f %f\n",f0,f1,f2);
						printf("#C(%f %f)\n",CEREAL(tmp0), IMMAG(tmp0));
						printf("#C(%f %f)\n",CEREAL(tmp1), IMMAG(tmp1));*/
						IMMAG(tmp0) = ((IMMAG(tmp0) * CEREAL(tmp1)) - (CEREAL(tmp0) * IMMAG(tmp1))) / f0; 
						CEREAL(tmp0) = ftmp;
						/*printf("#C(%f %f) %f\n",CEREAL(tmp0), IMMAG(tmp0), ftmp);*/
				}
				break;
		}
		rst = cdr(rst);
	}
	if(tmp0->object.n->type == RATIONAL)
	{
		itmp = _igcd(tmp0->object.n->nobject.rational.num,tmp0->object.n->nobject.rational.den);
		if(itmp != 1 && itmp != 0)
		{
			tmp0->object.n->nobject.rational.num /= itmp;
			tmp0->object.n->nobject.rational.den /= itmp;
		}
	}
	return tmp0;
}
SExp *
freal_part(SExp *tmp1)
{
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"real-part operates on numbers alone...");
	switch(NTYPE(tmp1))
	{
		case INTEGER:
		case REAL:
		case RATIONAL:
			return tmp1;
		case COMPLEX:
			return makereal(CEREAL(tmp1));
	}
}
SExp *
fimag_part(SExp *tmp1)
{
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"real-part operates on numbers alone...");
	switch(NTYPE(tmp1))
	{
		case INTEGER:
		case REAL:
		case RATIONAL:
			return tmp1;
		case COMPLEX:
			return makereal(IMMAG(tmp1));
	}	
}
SExp *
fmake_rect(SExp *tmp0, SExp *tmp1)
{
	SExp *tmp2 = nil;
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) == COMPLEX))
		return makeerror(1,0,"make-rectangular accepts only real arguments");
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) == COMPLEX))
		return makeerror(1,0,"make-rectangular accepts only real arguments");
	tmp2 = makenumber(INTEGER);
	NTYPE(tmp2) = COMPLEX;
	switch(NTYPE(tmp0))
	{
		case INTEGER:
			CEREAL(tmp2) = tmp0->object.n->nobject.z * 1.0;
			break;
		case REAL:
			CEREAL(tmp2) = tmp0->object.n->nobject.real;
			break;
		case RATIONAL:
			CEREAL(tmp2) = (NUM(tmp0) * 1.0) / (DEN(tmp0) / 1.0);
			break;
	}
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			IMMAG(tmp2) = tmp1->object.n->nobject.z * 1.0;
			break;
		case REAL:
			IMMAG(tmp2) = tmp1->object.n->nobject.real;
			break;
		case RATIONAL:
			IMMAG(tmp2) = (NUM(tmp1) * 1.0) / (DEN(tmp1) / 1.0);
			break;
	}
	return tmp2;
}
SExp *
fmake_pole(SExp *tmp0, SExp *tmp1)
{
	SExp *tmp2 = nil;
	double f0 = 0.0, f1 = 0.0;
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) == COMPLEX))
		return makeerror(1,0,"make-polar accepts only real arguments");
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) == COMPLEX))
		return makeerror(1,0,"make-polar accepts only real arguments");
	tmp2 = makenumber(INTEGER);
	NTYPE(tmp2) = COMPLEX;
	switch(NTYPE(tmp0))
	{
		case INTEGER:
			f0 = tmp0->object.n->nobject.z * 1.0 ;
			break;
		case REAL:
			f0 = tmp0->object.n->nobject.real;
			break;
		case RATIONAL:
			f0 = (NUM(tmp0) * 1.0) / (DEN(tmp0) / 1.0);
			break;
	}
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			f1 = tmp1->object.n->nobject.z * 1.0;
			break;
		case REAL:
			f1 = tmp1->object.n->nobject.real;
			break;
		case RATIONAL:
			f1 = (NUM(tmp1) * 1.0) / (DEN(tmp1) / 1.0);
			break;
	}
	CEREAL(tmp2) = f0 * cos(f1);
	IMMAG(tmp2) = f0 * sin(f1);
	return tmp2;
}
SExp *
fconjugate(SExp *tmp0)
{
	SExp *tmp1 = nil;
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) != COMPLEX))
		return makeerror(1,0,"conjugate accepts only complex numbers");
	tmp1 = makenumber(INTEGER);
	NTYPE(tmp1) = COMPLEX;
	CEREAL(tmp1) = CEREAL(tmp0);
	IMMAG(tmp1) = - IMMAG(tmp0);
	return tmp1;
}
SExp *
fconjugate_bang(SExp *tmp0)
{
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) != COMPLEX))
		return makeerror(1,0,"conjugate! accepts only complex numbers");
	IMMAG(tmp0) = - IMMAG(tmp0);
	return tmp0;
}
SExp *
fpol2rect(SExp *tmp0)
{
	SExp *tmp1 = nil;
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) != COMPLEX))
		return makeerror(1,0,"polar->rectangular's sole argument *must* be a complex number in polar format");
	tmp1 = makenumber(COMPLEX);
	CEREAL(tmp1) = (CEREAL(tmp0) * cosf(IMMAG(tmp0)));
	IMMAG(tmp1) = (CEREAL(tmp0) * sinf(IMMAG(tmp0)));
	return tmp1;
}
SExp *
frect2pol(SExp *tmp0)
{
	double f0 = 0.0, f1 = 0.0;
	SExp *tmp1 = nil;
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) != COMPLEX))
		return makeerror(1,0,"rectangular->polar's sole argument *must* be a complex number in polar format");
	tmp1 = makenumber(COMPLEX);
	f0 = CEREAL(tmp0);
	f0 *= f0;
	f1 = IMMAG(tmp0);
	f1 *= f1;
	CEREAL(tmp1) = sqrtf(f0 + f1);
	f0 = CEREAL(tmp0);
	f1 = IMMAG(tmp0);
	IMMAG(tmp1) = atanf(f1 / f0);
	return tmp1;
}
SExp *
fgcd(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = 0;
	if(pairlength(rst) <= 1)
		return makeerror(1,0,"gcd expects 2 or more arguments... ");
	tmp0 = makenumber(INTEGER);
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && (tmp1->object.n->type != INTEGER && tmp1->object.n->type != RATIONAL)))
		return makeerror(1,0,"gcd only operates on rationals...");
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
	}	
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER || (tmp1->type == NUMBER && (tmp1->object.n->type != INTEGER && tmp1->object.n->type != RATIONAL)))
			return makeerror(1,0,"gcd only operates on rationals...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.z = _igcd(tmp0->object.n->nobject.z, tmp1->object.n->nobject.z);
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num = _igcd(tmp0->object.n->nobject.rational.num, tmp1->object.n->nobject.z);
						itmp = tmp0->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.den = (itmp / _igcd(1,itmp));
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER: /* convert tmp0 to rational, fall through */
						tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z;
						tmp0->object.n->nobject.rational.den = 1;
						tmp0->object.n->type = RATIONAL;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num = _igcd(tmp0->object.n->nobject.rational.num,tmp1->object.n->nobject.rational.num);
						itmp = tmp1->object.n->nobject.rational.den;
						tmp0->object.n->nobject.rational.den = (itmp / _igcd(itmp,tmp0->object.n->nobject.rational.den)) * tmp0->object.n->nobject.rational.den;
						break;
				}
		}
		rst = cdr(rst);
	}
	return tmp0;
}
SExp *
flcm(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = 0;
	/* instead of having a dedicated lcm function, I use the fact that
	 * lcm(a,b) := (|a| / gcd(a,b)) * |b| ;
	 */
	if(pairlength(rst) <= 1)
		return makeerror(1,0,"gcd expects 2 or more arguments... ");
	tmp0 = makenumber(INTEGER);
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && (tmp1->object.n->type != INTEGER && tmp1->object.n->type != RATIONAL)))
		return makeerror(1,0,"lcm only operates on rationals...");
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
	}	
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER || (tmp1->type == NUMBER && (tmp1->object.n->type != INTEGER && tmp1->object.n->type != RATIONAL)))
			return makeerror(1,0,"lcm only operates on rationals...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						tmp0->object.n->nobject.z = (tmp0->object.n->nobject.z / _igcd(tmp0->object.n->nobject.z, tmp1->object.n->nobject.z)) * tmp1->object.n->nobject.z;
						break;
					case RATIONAL:
						tmp0->object.n->nobject.rational.num = (tmp0->object.n->nobject.rational.num / _igcd(tmp1->object.n->nobject.z,tmp0->object.n->nobject.rational.num)) * tmp1->object.n->nobject.z;
						tmp0->object.n->nobject.rational.den = _igcd(1,tmp0->object.n->nobject.rational.den);
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER: /* convert tmp0 to rational, fall through */
						tmp0->object.n->nobject.rational.num = tmp0->object.n->nobject.z;
						tmp0->object.n->nobject.rational.den = 1;
						tmp0->object.n->type = RATIONAL;
					case RATIONAL:
						tmp0->object.n->nobject.rational.den = _igcd(tmp0->object.n->nobject.rational.den,tmp1->object.n->nobject.rational.den);
						itmp = tmp1->object.n->nobject.rational.num;
						tmp0->object.n->nobject.rational.num = (itmp / _igcd(itmp,tmp0->object.n->nobject.rational.num)) * tmp0->object.n->nobject.rational.num;
						break;
				}
		}
		rst = cdr(rst);
	}
	return tmp0;
}
SExp *
fquotient(SExp *tmp, SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp->type != NUMBER || (tmp->type == NUMBER && NTYPE(tmp) == COMPLEX))
		return makeerror(1,0,"quotient (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) == COMPLEX))
		return makeerror(1,0,"quotient (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	tmp0 = makenumber(INTEGER);
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			switch(NTYPE(tmp))
			{
				case INTEGER:
					tmp0->object.n->nobject.z = tmp->object.n->nobject.z / tmp1->object.n->nobject.z;
					return tmp0;
				case RATIONAL:
					return tmp0;
			}
		case RATIONAL:
			return tmp0;
		case REAL:
			return tmp0;
	}
	return tmp0;
}
SExp *
fmodulo(SExp *tmp, SExp *tmp1)
{
	int itmp = 0;
	SExp *tmp0 = nil;
	if(tmp->type != NUMBER || (tmp->type == NUMBER && NTYPE(tmp) == COMPLEX))
		return makeerror(1,0,"modulo (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) == COMPLEX))
		return makeerror(1,0,"modulo (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	tmp0 = makenumber(INTEGER);
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			switch(NTYPE(tmp))
			{
				case INTEGER:
					tmp0->object.n->nobject.z = tmp->object.n->nobject.z % tmp1->object.n->nobject.z;
					return tmp0;
				case RATIONAL:
					itmp = (DEN(tmp0) / _igcd(1,DEN(tmp0)));
					printf("Floating after this?\n");
					NUM(tmp0) = (itmp * (NUM(tmp0) / DEN(tmp0))) % (itmp * tmp1->object.n->nobject.z);
					DEN(tmp0) = itmp;
					return tmp0;
			}
		case RATIONAL:
			return tmp0;
	}
	return tmp0;
}
SExp *
fremainder(SExp *tmp, SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp->type != NUMBER || (tmp->type == NUMBER && NTYPE(tmp) == COMPLEX))
		return makeerror(1,0,"remainder (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) == COMPLEX))
		return makeerror(1,0,"remainder (x0 : NUMBER) (x1 : NUMBER) => NUMBER ; note: x0 nor x1 may be complex");
	tmp0 = makenumber(INTEGER);
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			switch(NTYPE(tmp))
			{
				case INTEGER:
					tmp0->object.n->nobject.z = tmp->object.n->nobject.z % tmp1->object.n->nobject.z;
					return tmp0;
				case RATIONAL:
					return tmp0;
			}
	}
	return tmp0;
}
SExp *
fsin(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"sin operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = sin(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = sin(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = sin((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fcos(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"cos operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = cos(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = cos(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = cos((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
ftan(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"tan operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = tan(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = tan(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = tan((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fasin(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"asin operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = asin(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = asin(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = asin((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
facos(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"acos operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = acos(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = acos(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = acos((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fatan(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"atan operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = atan(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = atan(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = atan((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fatan2(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	double f0 = 0.0, f1 = 0.0;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"atan2 operates only on numbers...");
	if(tmp2->type != NUMBER)
		return makeerror(1,0,"atan2 operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			f0 = tmp1->object.n->nobject.z * 1.0;
			break;
		case REAL:
			f0 = tmp1->object.n->nobject.real;
			break;
		case RATIONAL:
			f0 = (NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0);
			break;
		case COMPLEX:
			break;
	}
	switch(NTYPE(tmp2))
	{
		case INTEGER:
			f1 = tmp2->object.n->nobject.z * 1.0;
			break;
		case REAL:
			f1 = tmp2->object.n->nobject.real;
			break;
		case RATIONAL:
			f1 = (NUM(tmp2) * 1.0) / (DEN(tmp2) * 1.0);
			break;
		case COMPLEX:
			break;
	}
	tmp0->object.n->nobject.real = atan2(f0,f1);
	return tmp0;
}
SExp *
fcosh(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"cosh operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = cosh(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = cosh(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = cosh((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fsinh(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"sinh operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = sinh(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = sinh(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = sinh((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
ftanh(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"tanh operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = tanh(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = tanh(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = tanh((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fexp(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"exp operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = exp(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = exp(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = exp((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fexp2(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"exp2 operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER: /* this should return an exact... */
			NTYPE(tmp0) = INTEGER;
			tmp0->object.n->nobject.z = 1 << tmp1->object.n->nobject.z;
			//tmp0->object.n->nobject.real = exp2f(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = exp2(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = exp2((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fexpm1(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"expm1 operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = expm1(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = expm1(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = expm1((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fln(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"ln operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = log(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = log(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = log((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
flog2(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"log2 operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = log2(tmp1->object.n->nobject.z * 1.0l);
			break;
		case REAL:
			tmp0->object.n->nobject.real = log2(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = log2((NUM(tmp1) * 1.0l) / (DEN(tmp1) * 1.0l));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
flog10(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"log10 operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	NTYPE(tmp0) = REAL;
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.real = log10(tmp1->object.n->nobject.z * 1.0);
			break;
		case REAL:
			tmp0->object.n->nobject.real = log10(tmp1->object.n->nobject.real);
			break;
		case RATIONAL:
			tmp0->object.n->nobject.real = log10((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0));
			break;
		case COMPLEX:
			break;
	}
	return tmp0;
}
SExp *
fnabs(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"abs operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z < 0 ? - tmp1->object.n->nobject.z : tmp1->object.n->nobject.z;
			NTYPE(tmp0) = INTEGER;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real < 0 ? - tmp1->object.n->nobject.real : tmp1->object.n->nobject.real;
			NTYPE(tmp0) = REAL;
			break;
		case RATIONAL:
			NUM(tmp0) = NUM(tmp1) < 0 ? - NUM(tmp1) : NUM(tmp1);
			DEN(tmp0) = DEN(tmp1) < 0 ? - DEN(tmp1) : DEN(tmp1);
			NTYPE(tmp0) = RATIONAL;
			break;
		case COMPLEX: /* should this do something? magnitude is different... */
			return tmp1;
	}
	return tmp0;
}
SExp *
fmag(SExp *tmp1)
{
	SExp *tmp0 = nil;
	double f0 = 0.0, f1 = 0.0;
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"abs operates only on numbers...");
	tmp0 = makenumber(INTEGER);
	switch(NTYPE(tmp1))
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z < 0 ? - tmp1->object.n->nobject.z : tmp1->object.n->nobject.z;
			NTYPE(tmp0) = INTEGER;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real < 0 ? - tmp1->object.n->nobject.real : tmp1->object.n->nobject.real;
			NTYPE(tmp0) = REAL;
			break;
		case RATIONAL:
			NUM(tmp0) = NUM(tmp1) < 0 ? - NUM(tmp1) : NUM(tmp1);
			DEN(tmp0) = DEN(tmp1) < 0 ? - DEN(tmp1) : DEN(tmp1);
			NTYPE(tmp0) = RATIONAL;
			break;
		case COMPLEX:
			NTYPE(tmp0) = REAL;
			f0 = CEREAL(tmp1);
			f0 *= f0;
			f1 = IMMAG(tmp1);
			f1 *= f1;
			tmp0->object.n->nobject.real = sqrtf(f0 + f1);
			break;
	}
	return tmp0;
}
SExp *
flt(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	if(itmp < 1)
		return makeerror(1,0,"< expects at least one argument...");
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"< operates on numbers only...");
	if(itmp == 1)
		return strue;
	tmp0 = makenumber(INTEGER);
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			tmp0->object.n->type = REAL;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
		case COMPLEX:
			tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
			tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
			tmp0->object.n->type = COMPLEX;
			break;
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"< operates on numbers only...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z >= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real >= (tmp1 ->object.n->nobject.z * 1.0))
							return sfalse;
						break;
					case RATIONAL:
						itmp = (tmp0->object.n->nobject.rational.num / tmp0->object.n->nobject.rational.den);
						if(itmp >= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) >= (tmp1->object.n->nobject.z * 1.0))
							return sfalse;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) >= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real >= tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if( ((tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0)) >= tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) >= tmp1->object.n->nobject.real)
							return sfalse;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						itmp = (tmp1->object.n->nobject.rational.num / tmp1->object.n->nobject.rational.den);
						if(tmp0->object.n->nobject.z >= itmp)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real >= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
					case RATIONAL:
						/* this is a loss of precision issue; there is a better way than upconverting to REAL;
						 * however, this is quite easy to do for PoC. FIXME later...
						 */
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) >= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;	
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(IMMAG(tmp0) >= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
				}
				break;
			case COMPLEX:
				/*printf("IMMAG(tmp1) == 0.0? %s\n", IMMAG(tmp1) == 0.0 ? "#t" : "#f");
				printf("%f\n",IMMAG(tmp1));*/
				if(NTYPE(tmp0) != COMPLEX && IMMAG(tmp1) < 0.0) /* up conversion... */
					return sfalse;
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) >= CEREAL(tmp1))
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real >= CEREAL(tmp1))
							return sfalse;
						break;
					case RATIONAL:
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) >= CEREAL(tmp1))
							return sfalse;
						break;
					case COMPLEX:
						if(CEREAL(tmp0) >= CEREAL(tmp1))
							return sfalse;
						if(IMMAG(tmp0) >= IMMAG(tmp1))
							return sfalse;
						break;
				}
				break;
		}
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
				tmp0->object.n->type = INTEGER;
				break;
			case REAL:
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
				tmp0->object.n->type = REAL;
				break;
			case RATIONAL:
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
				tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
				tmp0->object.n->type = RATIONAL;
				break;
			case COMPLEX:
				tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
				tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
				tmp0->object.n->type = COMPLEX;
				break;
		}
		rst = cdr(rst);
	}
	return strue;
}
SExp *
flte(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	if(itmp < 1)
		return makeerror(1,0,"<= expects at least one argument...");
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"<= operates on numbers only...");
	if(itmp == 1)
		return strue;
	tmp0 = makenumber(INTEGER);
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			tmp0->object.n->type = REAL;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
		case COMPLEX:
			tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
			tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
			tmp0->object.n->type = COMPLEX;
			break;
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"<= operates on numbers only...");
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z > tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real > (tmp1 ->object.n->nobject.z * 1.0))
							return sfalse;
						break;
					case RATIONAL:
						itmp = (tmp0->object.n->nobject.rational.num / tmp0->object.n->nobject.rational.den);
						if(itmp > tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) > (tmp1->object.n->nobject.z * 1.0))
							return sfalse;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) > tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real > tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if( ((tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0)) > tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) > tmp1->object.n->nobject.real)
							return sfalse;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						itmp = (tmp1->object.n->nobject.rational.num / tmp1->object.n->nobject.rational.den);
						if(tmp0->object.n->nobject.z > itmp)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real > ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
					case RATIONAL:
						/* this is a loss of precision issue; there is a better way than upconverting to REAL;
						 * however, this is quite easy to do for PoC. FIXME later...
						 */
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) > ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;	
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(IMMAG(tmp0) > ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
				}
				break;
			case COMPLEX:
				/*printf("IMMAG(tmp1) == 0.0? %s\n", IMMAG(tmp1) == 0.0 ? "#t" : "#f");
				printf("%f\n",IMMAG(tmp1));*/
				if(NTYPE(tmp0) != COMPLEX && IMMAG(tmp1) < 0.0) /* up conversion... */
					return sfalse;
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) > CEREAL(tmp1))
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real > CEREAL(tmp1))
							return sfalse;
						break;
					case RATIONAL:
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) > CEREAL(tmp1))
							return sfalse;
						break;
					case COMPLEX:
						if(CEREAL(tmp0) > CEREAL(tmp1))
							return sfalse;
						if(IMMAG(tmp0) > IMMAG(tmp1))
							return sfalse;
						break;
				}
				break;
		}
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
				tmp0->object.n->type = INTEGER;
				break;
			case REAL:
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
				tmp0->object.n->type = REAL;
				break;
			case RATIONAL:
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
				tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
				tmp0->object.n->type = RATIONAL;
				break;
			case COMPLEX:
				tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
				tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
				tmp0->object.n->type = COMPLEX;
				break;
		}
		rst = cdr(rst);
	}
	return strue;
}
SExp *
fgt(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	itmp = pairlength(rst);
	if(itmp < 1)
		return makeerror(1,0,"> expects at least one argument...");
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"> operates on numbers only...");
	if(itmp == 1)
		return strue;
	tmp0 = makenumber(INTEGER);
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			tmp0->object.n->type = REAL;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
		case COMPLEX:
			tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
			tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
			tmp0->object.n->type = COMPLEX;
			break;
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,"> operates on numbers only...");
		/*printf("tmp0:tmp1 => ");
		princ(tmp0);
		printf(" : ");
		princ(tmp1);
		printf("\n");*/
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z <= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real <= (tmp1 ->object.n->nobject.z * 1.0))
							return sfalse;
						break;
					case RATIONAL:
						itmp = (tmp0->object.n->nobject.rational.num / tmp0->object.n->nobject.rational.den);
						if(itmp <= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) >= 0.0)
							return sfalse;
						if(CEREAL(tmp0) <= (tmp1->object.n->nobject.z * 1.0))
							return sfalse;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) <= tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real <= tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if( ((tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0)) <= tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) <= 0.0)
							return sfalse;
						if(CEREAL(tmp0) <= tmp1->object.n->nobject.real)
							return sfalse;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						itmp = (tmp1->object.n->nobject.rational.num / tmp1->object.n->nobject.rational.den);
						if(tmp0->object.n->nobject.z <= itmp)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real <= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
					case RATIONAL:
						/* this is a loss of precision issue; there is a better way than upconverting to REAL;
						 * however, this is quite easy to do for PoC. FIXME later...
						 */
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) <= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;	
						break;
					case COMPLEX:
						if(IMMAG(tmp0) <= 0.0)
							return sfalse;
						if(IMMAG(tmp0) <= ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
				}
				break;
			case COMPLEX:
				/*printf("IMMAG(tmp1) == 0.0? %s\n", IMMAG(tmp1) == 0.0 ? "#t" : "#f");
				printf("%f\n",IMMAG(tmp1));*/
				if(NTYPE(tmp0) != COMPLEX && IMMAG(tmp1) <= 0.0) /* up conversion... */
					return sfalse;
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) <= CEREAL(tmp1))
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real <= CEREAL(tmp1))
							return sfalse;
						break;
					case RATIONAL:
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) <= CEREAL(tmp1))
							return sfalse;
						break;
					case COMPLEX:
						if(CEREAL(tmp0) <= CEREAL(tmp1))
							return sfalse;
						if(IMMAG(tmp0) <= IMMAG(tmp1))
							return sfalse;
						break;
				}
				break;
		}
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
				tmp0->object.n->type = INTEGER;
				break;
			case REAL:
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
				tmp0->object.n->type = REAL;
				break;
			case RATIONAL:
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
				tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
				tmp0->object.n->type = RATIONAL;
				break;
			case COMPLEX:
				tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
				tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
				tmp0->object.n->type = COMPLEX;
				break;
		}
		rst = cdr(rst);
	}
	return strue;
}
SExp *
fgte(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	if(itmp < 1)
		return makeerror(1,0,">= expects at least one argument...");
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,">= operates on numbers only...");
	if(itmp == 1)
		return strue;
	tmp0 = makenumber(INTEGER);
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			tmp0->object.n->type = REAL;
			break;
		case RATIONAL:
			tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
			tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
			tmp0->object.n->type = RATIONAL;
			break;
		case COMPLEX:
			tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
			tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
			tmp0->object.n->type = COMPLEX;
			break;
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER)
			return makeerror(1,0,">= operates on numbers only...");
		/*printf("tmp0:tmp1 => ");
		princ(tmp0);
		printf(" : ");
		princ(tmp1);
		printf("\n");*/
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z < tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real < (tmp1 ->object.n->nobject.z * 1.0))
							return sfalse;
						break;
					case RATIONAL:
						itmp = (tmp0->object.n->nobject.rational.num / tmp0->object.n->nobject.rational.den);
						if(itmp < tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) < (tmp1->object.n->nobject.z * 1.0))
							return sfalse;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) < tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
					  if(tmp0->object.n->nobject.real < tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if( ((tmp0->object.n->nobject.rational.num * 1.0) / (tmp0->object.n->nobject.rational.den * 1.0)) < tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(CEREAL(tmp0) < tmp1->object.n->nobject.real)
							return sfalse;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						itmp = (tmp1->object.n->nobject.rational.num / tmp1->object.n->nobject.rational.den);
						if(tmp0->object.n->nobject.z < itmp)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real < ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
					case RATIONAL:
						/* this is a loss of precision issue; there is a better way than upconverting to REAL;
						 * however, this is quite easy to do for PoC. FIXME later...
						 */
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) < ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;	
						break;
					case COMPLEX:
						if(IMMAG(tmp0) < 0.0)
							return sfalse;
						if(IMMAG(tmp0) < ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
				}
				break;
			case COMPLEX:
				/*printf("IMMAG(tmp1) == 0.0? %s\n", IMMAG(tmp1) == 0.0 ? "#t" : "#f");
				printf("%f\n",IMMAG(tmp1));*/
				if(NTYPE(tmp0) != COMPLEX && IMMAG(tmp1) < 0.0) /* up conversion... */
					return sfalse;
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((tmp0->object.n->nobject.z * 1.0) < CEREAL(tmp1))
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real < CEREAL(tmp1))
							return sfalse;
						break;
					case RATIONAL:
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) < CEREAL(tmp1))
							return sfalse;
						break;
					case COMPLEX:
						if(CEREAL(tmp0) < CEREAL(tmp1))
							return sfalse;
						if(IMMAG(tmp0) < IMMAG(tmp1))
							return sfalse;
						break;
				}
				break;
		}
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
				tmp0->object.n->type = INTEGER;
				break;
			case REAL:
				tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
				tmp0->object.n->type = REAL;
				break;
			case RATIONAL:
				tmp0->object.n->nobject.rational.num = tmp1->object.n->nobject.rational.num;
				tmp0->object.n->nobject.rational.den = tmp1->object.n->nobject.rational.den;
				tmp0->object.n->type = RATIONAL;
				break;
			case COMPLEX:
				tmp0->object.n->nobject.complex.r = tmp1->object.n->nobject.complex.r;
				tmp0->object.n->nobject.complex.i = tmp1->object.n->nobject.complex.i;
				tmp0->object.n->type = COMPLEX;
				break;
		}
		rst = cdr(rst);
	}
	return strue;
}
SExp *
fnumeq(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	if(itmp < 1)
		return makeerror(1,0,"= expects at least one argument...");
	tmp1 = car(rst);
	rst = cdr(rst);
	if(tmp1->type != NUMBER)
		return makeerror(1,0,"= operates on numbers only...");
	else if(itmp == 1)
		return strue;
	tmp0 = makenumber(INTEGER);
	switch(tmp1->object.n->type)
	{
		case INTEGER:
			tmp0->object.n->nobject.z = tmp1->object.n->nobject.z;
			break;
		case REAL:
			tmp0->object.n->nobject.real = tmp1->object.n->nobject.real;
			tmp0->object.n->type = REAL;
			break;
		case RATIONAL:
			NUM(tmp0) = NUM(tmp1);
			DEN(tmp0) = DEN(tmp1);
			tmp0->object.n->type = RATIONAL;
			break;
		case COMPLEX:
			CEREAL(tmp0) = CEREAL(tmp1);
			IMMAG(tmp0) = IMMAG(tmp1);
			tmp0->object.n->type = COMPLEX;
			break;
	}
	while(rst != snil)
	{
		tmp1 = car(rst);
		switch(tmp1->object.n->type)
		{
			case INTEGER:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z != tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real != (1.0 * tmp1->object.n->nobject.z))
							return sfalse;
						break;
					case RATIONAL:
						if((NUM(tmp0)/DEN(tmp0)) != tmp1->object.n->nobject.z)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) != 0.0)
							return sfalse;
						if(CEREAL(tmp0) != (1.0 * tmp1->object.n->nobject.z))
							return sfalse;
						break;
				}
				break;
			case REAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if((1.0 * tmp0->object.n->nobject.z) != tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real != tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if(((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)) != tmp1->object.n->nobject.real)
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) != 0.0)
							return sfalse;
						if(CEREAL(tmp0) != tmp1->object.n->nobject.real)
							return sfalse;
						break;
				}
				break;
			case RATIONAL:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(tmp0->object.n->nobject.z != (NUM(tmp1) / DEN(tmp1)))
							return sfalse;
						break;
					case REAL:
						if(tmp0->object.n->nobject.real != ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
					case RATIONAL:
						if(NUM(tmp1) != NUM(tmp0))
							return sfalse;
						if(DEN(tmp1) != DEN(tmp0))
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp0) != 0.0)
							return sfalse;
						if(CEREAL(tmp0) != ((NUM(tmp1) * 1.0) / (DEN(tmp1) * 1.0)))
							return sfalse;
						break;
				}
				break;
			case COMPLEX:
				switch(tmp0->object.n->type)
				{
					case INTEGER:
						if(IMMAG(tmp1) != 0.0)
							return sfalse;
						if(CEREAL(tmp1) != (1.0 * tmp0->object.n->nobject.z))
							return sfalse;
						break;
					case REAL:
						if(IMMAG(tmp1) != 0.0)
							return sfalse;
						if(CEREAL(tmp1) != tmp0->object.n->nobject.real)
							return sfalse;
						break;
					case RATIONAL:
						if(IMMAG(tmp1) != 0.0)
							return sfalse;
						if(CEREAL(tmp1) != ((NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0)))
							return sfalse;
						break;
					case COMPLEX:
						if(IMMAG(tmp1) != IMMAG(tmp0))
							return sfalse;
						if(CEREAL(tmp1) != CEREAL(tmp0))
							return sfalse;
						break;
				}
				break;
			}
		rst = cdr(rst);
	}
	return strue;
}
/* collection functions */
SExp *
ffirst(SExp *tmp0)
{
	switch(tmp0->type)
	{
		case VECTOR:
			if(tmp0->length < 1)
				return snil;
			return tmp0->object.vec[0];
		case STRING:
			if(tmp0->length >= 1)
				return makechar(tmp0->object.str[0]);
			return snil;
		case PAIR:
			if(tmp0 != snil)
				return car(tmp0);
			return snil;
		case DICT:
			/* nothing for now... */
			return snil;
		default:
			return makeerror(1,0,"first operates on collections only...");
	}
}
SExp *
frest(SExp *tmp0)
{
	int itmp = 0, iter = 0;
	SExp *tmp1 = nil;
	switch(tmp0->type)
	{
		case VECTOR:
			tmp1 = makevector(tmp0->length - 1,snil);
			itmp = tmp0->length;
			for(iter = 1;iter < itmp;iter++)
				tmp1->object.vec[iter - 1] = tmp0->object.vec[iter];
			return tmp1;
		case STRING:
			return makestring(&tmp0->object.str[1]);
		case PAIR:
			return cdr(tmp0);
		case DICT:
			return snil;
		default:
			return makeerror(1,0,"rest operates on collections only...");
	}
}
SExp *
fnth(SExp *tmp0, SExp *tmp1)
{
	int iter = 0, itmp = 0;
	SExp *tmp2 = snil;
	if(tmp0->type != DICT)
	{
		if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
			return makeerror(1,0,"nth: idx must be an INTEGER");
	}
	else
		if(tmp1->type != STRING && tmp1->type != KEY && tmp1->type != ATOM)
			return makeerror(1,0,"nh: idx for dicts must be bound to (STRING | KEYOBJ | SYMBOL)");
	switch(tmp0->type)
	{
		case VECTOR:
			if(tmp1->object.n->nobject.z >= tmp0->length)
				return makeerror(1,0,"nth: index out of range");
			return tmp0->object.vec[tmp1->object.n->nobject.z];
		case STRING:
			if(tmp1->object.n->nobject.z >= tmp0->length)
				return makeerror(1,0,"nth: index out of range");
			return makechar(tmp0->object.str[tmp1->object.n->nobject.z]);
		case PAIR:
			itmp = pairlength(tmp0);
			if(tmp1->object.n->nobject.z > itmp)
				return makeerror(1,0,"nth: index out of range");
			for(iter = 0; tmp0 != snil; iter++)
			{
				if(iter == tmp1->object.n->nobject.z)
					return car(tmp0);
				tmp0 = cdr(tmp0);
			}
			return snil;
		case DICT:
			tmp2 = trie_get(tmp1->object.str,tmp0->object.dict);
			if(tmp2 == nil)
				return makeerror(1,0,"No such key");
			return tmp2;
		default: 
			return makeerror(1,0,"first operates on collections only...");
	}
}
SExp *
fcset(SExp *col, SExp *offset, SExp *new)
{
	int iter = 0, itmp = 0;
	SExp *tmp = nil;
	switch(col->type)
	{
		case STRING:
			if(offset->type != NUMBER || (offset->type == NUMBER && NTYPE(offset) != INTEGER))
				return makeerror(1,0,"cset!: strings are indexed by FIXNUM integers only.");
			if(AINT(offset) > col->length)
				return makeerror(1,0,"cset!: string index out of range.");
			if(new->type != CHAR)
				return makeerror(1,0,"cset!: strings may only be updated with characters.");
			col->object.str[AINT(offset)] = new->object.c;
			break;
		case VECTOR:
			if(offset->type != NUMBER || (offset->type == NUMBER && NTYPE(offset) != INTEGER))
				return makeerror(1,0,"cset!: vectors are indexed by FIXNUM integers only.");
			if(AINT(offset) > col->length)
				return makeerror(1,0,"cset!: vector index out of range.");
			col->object.vec[AINT(offset)] = new;
			break;
		case DICT:
			if(offset->type != STRING && offset->type != ATOM && offset->type != KEY)
				return makeerror(1,0,"cset!: dictionary keys must be (STRING | ATOM | KEY)");
			trie_put(offset->object.str,new,col->object.dict);
			break;
		case PAIR:
			tmp = col;
			itmp = pairlength(tmp);
			if(offset->type != NUMBER || (offset->type == NUMBER && NTYPE(offset) != INTEGER))
				return makeerror(1,0,"cset!: pairs are indexed by FIXNUM integers only.");
			if(AINT(offset) > itmp)
				return makeerror(1,0,"cset!: pair index out of range");
			for(itmp = AINT(offset);iter < itmp;iter++)
				tmp = cdr(tmp);
			mcar(tmp) = new;
			break;
	}
	return svoid;
}
SExp *
fccons(SExp *tmp0, SExp *tmp1)
{
	SExp *tmp2 = nil;
	int iter = 0, itmp = 0;
	switch(tmp1->type)
	{
		case VECTOR:
			tmp2 = makevector(tmp1->length + 1,snil);
			for(iter = 0;iter < tmp1->length; iter++)
				tmp2->object.vec[iter + 1] = tmp1->object.vec[iter];
			tmp2->object.vec[0] = tmp0;
			return tmp2;
		case STRING:
			if(tmp0->type != CHAR)
				return makeerror(1,0,"type clash: ccons may only add chars to strings...");
			tmp2 = makestring_v(tmp1->length + 1,' ');
			for(iter = 0; iter < tmp1->length; iter++)
				tmp2->object.str[iter + 1] = tmp1->object.str[iter];
			tmp2->object.str[0] = tmp0->object.c;
			return tmp2;
		case NIL:
		case PAIR:
			return cons(tmp0,tmp1);
		case DICT:
			return snil;
		default:
			return makeerror(1,0,"ccons operates on collections only...");
	}
}
SExp *
flength(SExp *tmp1)
{
	SExp *tmp0 = makenumber(INTEGER);
	if(tmp1->type == VECTOR || tmp1->type == STRING)
	{
		tmp0->object.n->nobject.z = tmp1->length;
		return tmp0;
	}
	else if(tmp1->type == PAIR)
	{
		tmp0->object.n->nobject.z = pairlength(tmp1);
		return tmp0;
	}
	else if(tmp1->type == NIL)
		return makeinteger(0);
	/* would it be meaningful to return 0 here?
	 * technically, scalars have a zero length...
	 */
	return makeerror(1,0,"type clash...");
}
SExp *
fempty(SExp *tmp0)
{
	if(tmp0->type == VECTOR || tmp0->type == STRING)
	{
		if(tmp0->length > 0)
			return sfalse;
		return strue;
	}
	else if(tmp0->type == PAIR)
		return sfalse;
	else if(tmp0 == snil)
		return strue;
	return sfalse;
}
SExp *
fcupdate(SExp *col, SExp *index, SExp *nuval)
{
	SExp *ret = nil;
	int i = 0;
	/* should start testing this in every subroutine... */
	if(col == nil)
		return nil;
	if(col->type == STRING)
	{
		if(index->type != NUMBER || (index->type == NUMBER && NTYPE(index) != INTEGER))
			return makeerror(1,0,"cupdate: index error: strings *must* be indexed by integers");
		if(AINT(index) > col->length)
			return makeerror(1,0,"cupdate: index out of range");
		if(nuval->type != CHAR)
			return makeerror(1,0,"cupdate: new-value error: the new vaule *must* be a character");
		ret = makestring(col->object.str);
		ret->object.str[AINT(index)] = nuval->object.c;
		return ret;
	}
	else if(col->type == VECTOR)
	{
		if(index->type != NUMBER || (index->type == NUMBER && NTYPE(index) != INTEGER))
			return makeerror(1,0,"cupdate: index error: strings *must* be indexed by integers");
		if(AINT(index) > col->length)
			return makeerror(1,0,"cupdate: index out of range");
		ret = makevector(col->length,nil);
		for(i = 0; i < col->length; i++)
		{
			if(i == AINT(index))
				ret->object.vec[i] = nuval;
			else
				ret->object.vec[i] = col->object.vec[i];
		}
		return ret;
	}
	else if(col->type == DICT)
	{
		return snil;
	}
	else if(col->type == PAIR)
	{
		return snil;
	}
	return makeerror(1,0,"cupdate: col *must* be bound to a collexion");
}
SExp *
fcslice(SExp *col, SExp *start, SExp *end)
{
	SExp *ret = nil;
	int i = 0, j = 0, base = 0;
	if(col->type != VECTOR && col->type != DICT && col->type != STRING && col->type != PAIR)
		return makeerror(1,0,"cslice's col argument *must* be bound to a collexion");
	if(start->type != NUMBER || (start->type == NUMBER && NTYPE(start) != INTEGER))
		return makeerror(1,0,"cslice's start argument *must* be bound to an integer");
	if(end->type != NUMBER || (end->type == NUMBER && NTYPE(end) != INTEGER))
		return makeerror(1,0,"cslice's end argument *must* be bound to an integer");
	switch(col->type)
	{
		case STRING:
			j = AINT(end);
			if(j < 0)
				j = col->length + j + 1;
			i = AINT(start);
			if(i < 0)
				i = col->length + i;
			/* should probably have it where if you say cslice(col,-1,4), it copies the 
			 * end & wraps to 4, but this seems to be an edge-case that won' be oft used...
			 */
			 if(i < 0)
			 	return makeerror(1,0,"cslice's start argument *must* be greater than 0");
			 ret = makestring_v(j,nul);
			 ret->length = j;
			 for(;i < j;i++, base++)
			 	ret->object.str[base] = col->object.str[i];
			 ret->object.str[base] = nul;
			 break;	
		case VECTOR:
		case PAIR:
		case DICT:
			break;
	}
	return ret;
}
SExp *
fvector(SExp *rst)
{
	SExp *tmp0 = nil;
	int itmp = pairlength(rst), iter = 0;
	if(itmp < 1)
		return makevector(0,snil);
	tmp0 = makevector(itmp,nil);
	for(iter = 0; iter < itmp; iter++)
	{
		tmp0->object.vec[iter] = car(rst);
		rst = cdr(rst);
	}
	return tmp0;
}
SExp *
fmkvector(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	int itmp = pairlength(rst);
	if(itmp < 1 || itmp > 2)
		return makeerror(1,0,"make-vector requires at least one argument (and at most two)");
	if(itmp == 1)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
			return makeerror(1,0,"make-vector's first argument must be an integer...");
		tmp0 = makevector(tmp1->object.n->nobject.z,snil);
	}
	else if(itmp == 2)
	{
		tmp1 = car(rst);
		if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
			return makeerror(1,0,"make-vector's first argument must be an integer...");
		tmp2 = car(cdr(rst));
		if(tmp2->type == KEY && !strncasecmp(tmp2->object.str,"no-init",7))
			tmp0 = makevector(tmp1->object.n->nobject.z,nil); /* DO NOT INIT; pretty dangerous, but useful for "pure speed"*/
		else
			tmp0 = makevector(tmp1->object.n->nobject.z,tmp2);
	}
	return tmp0;
}
SExp *
fstringappend(SExp *rst)
{
	/* O(scary) */
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	int itmp = pairlength(rst), stmp = 0, iter = 0;
	if(itmp == 0)
		return makestring_v(0,' ');
	tmp0 = rst;
	itmp = 0;
	while(tmp0 != snil)
	{
		tmp1 = car(tmp0);
		if(tmp1->type != STRING)
			return makeerror(1,0,"string-append operates on STRINGs only!");
		itmp += tmp1->length;
		tmp0 = cdr(tmp0);
	}
	tmp0 = rst;
	tmp1 = makestring_v(itmp,nul); /* don't fill! */
	iter = 0;
	while(tmp0 != snil)
	{
		tmp2 = car(tmp0);
		stmp = iter;
		itmp = tmp2->length;
		for(;(iter - stmp) < itmp; iter++)
			tmp1->object.str[iter] = tmp2->object.str[iter - stmp];
		tmp0 = cdr(tmp0);
	}
	tmp1->object.str[iter] = nul;
	return tmp1;
}
SExp *
fdict(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	tmp0 = makedict();
  	while(rst != snil)
  	{
  		tmp1 = car(rst);
  		if(tmp1->type != STRING && tmp1->type != ATOM && tmp1->type != KEY)
  			return makeerror(1,0,"keys to dict *must* be (STRING | SYMBOL | KEYOBJ)");
  		if(cdr(rst) != snil)
  			tmp2 = car(cdr(rst));
  		else
  			tmp2 = snil;
  		trie_put(tmp1->object.str,tmp2,tmp0->object.dict);
  		rst = cdr(cdr(rst));
  	}	
	return tmp0;
}
SExp *
fdicthas(SExp *tmp0, SExp *tmp1)
{
	SExp *tmp2 = nil;
	if(tmp0->type != DICT)
		return makeerror(1,0,"dict-has?'s d argument *must* be bound to a dictionary");
	if(tmp1->type != STRING && tmp1->type != KEY && tmp1->type != ATOM)
		return makeerror(1,0,"dict-has?'s k object *must* be bound to (KEYOBJ | STRING | ATOM)");
	tmp2 = trie_get(tmp1->object.str,tmp0->object.dict);
	if(tmp2 == nil)
		return sfalse;
	return strue;
}
SExp *
fstring(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst), iter = 0;
	if(itmp == 0)
		return makestring_v(0,nul);
	tmp0 = makestring_v(itmp,' ');
	while(rst != snil)
	{
		tmp1 = car(rst);
		if(tmp1->type != CHAR)
			return makeerror(1,0,"string (c* : CHARACTER) => string");
		tmp0->object.str[iter] = tmp1->object.c;
		iter++;
		rst = cdr(rst);
	}
	return tmp0;
}
SExp *
fmakestring(SExp *rst)
{
	SExp *tmp0 = nil, *tmp1 = nil;
	int itmp = pairlength(rst);
	switch(itmp)
	{
		case 1:
			tmp1 = car(rst);
			if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
				return makeerror(1,0,"makestring (length : INTEGER) [(fill : CHARACTER)] => string");
			return makestring_v(tmp1->object.n->nobject.z,' ');
		case 2:
			tmp1 = car(rst);
			if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
				return makeerror(1,0,"makestring (length : INTEGER) [(fill : CHARACTER)] => string");
			tmp0 = car(cdr(rst));
			if(tmp0->type != CHAR)
				return makeerror(1,0,"makestring (length : INTEGER) [(fill : CHARACTER)] => string");
			return makestring_v(tmp1->object.n->nobject.z,tmp0->object.c);
		default:
			return makeerror(1,0,"makestring (length : INTEGER) [(fill : CHARACTER)] => string");
	}
}
/* generic internal functions */
SExp *
fgensym(SExp *tmp0)
{
	SExp *tmp1 = nil;
	char *buf = nil;
	if(tmp0 == snil)
	{
		buf = (char *)hmalloc(sizeof(char) * 16);
		snprintf(buf,16,"g%d",gensymglobal);
	}
	else
	{
		/* couldn't this just as easily be a key or a string? */
		if(tmp0->type != ATOM && tmp0->type != STRING)
			return makeerror(1,0,"gensym [a : (ATOM | STRING)] => symbol");
		buf = (char *)hmalloc(sizeof(char) * (tmp0->length + 16));
		snprintf(buf,tmp0->length + 16,"%s%d",tmp0->object.str,gensymglobal);
	}
	gensymglobal++;
	tmp1 = (SExp *)hmalloc(sizeof(SExp));
	tmp1->object.str = buf;
	tmp1->type = ATOM;
	return tmp1;
}
SExp *
fbitand(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,"& expects exactly two integer arguments");
	if(tmp2->type != NUMBER || (tmp2->type == NUMBER && NTYPE(tmp2) != INTEGER))
		return makeerror(1,0,"& expects exactly two integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = tmp1->object.n->nobject.z & tmp2->object.n->nobject.z;
	return tmp0;
}
SExp *
fbitor(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,"| expects exactly two integer arguments");
	if(tmp2->type != NUMBER || (tmp2->type == NUMBER && NTYPE(tmp2) != INTEGER))
		return makeerror(1,0,"& expects exactly two integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = tmp1->object.n->nobject.z | tmp2->object.n->nobject.z;
	return tmp0;
}
SExp *fbitxor(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,"& expects exactly two integer arguments");
	if(tmp2->type != NUMBER || (tmp2->type == NUMBER && NTYPE(tmp2) != INTEGER))
		return makeerror(1,0,"& expects exactly two integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = tmp1->object.n->nobject.z ^ tmp2->object.n->nobject.z;
	return tmp0;
}
SExp *
fbitnot(SExp *tmp1)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,"& expects exactly one integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = ~ tmp1->object.n->nobject.z;
	return tmp0;
}
SExp *
fbitshl(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,"<< expects exactly two integer arguments");
	if(tmp2->type != NUMBER || (tmp2->type == NUMBER && NTYPE(tmp2) != INTEGER))
		return makeerror(1,0,"<< expects exactly two integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = (unsigned int) tmp1->object.n->nobject.z << (unsigned int)tmp2->object.n->nobject.z;
	return tmp0;
}
SExp *
fbitshr(SExp *tmp1, SExp *tmp2)
{
	SExp *tmp0 = nil;
	if(tmp1->type != NUMBER || (tmp1->type == NUMBER && NTYPE(tmp1) != INTEGER))
		return makeerror(1,0,">> expects exactly two integer arguments");
	if(tmp2->type != NUMBER || (tmp2->type == NUMBER && NTYPE(tmp2) != INTEGER))
		return makeerror(1,0,">> expects exactly two integer arguments");
	tmp0 = makenumber(INTEGER);
	tmp0->object.n->nobject.z = (unsigned int)tmp1->object.n->nobject.z >> (unsigned int)tmp2->object.n->nobject.z;
	return tmp0;
}
SExp *
fceil(SExp *tmp0)
{
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) == COMPLEX))
		return makeerror(1,0,"ceil's r argument must be bound to a Number in the set or REALs");
	switch(NTYPE(tmp0))
	{
		case INTEGER:
			return tmp0;
		case REAL:
			return makereal(ceil(AREAL(tmp0)));
		case RATIONAL:
			return makeinteger(1);
	}
}
SExp *
ffloor(SExp *tmp0)
{
	if(tmp0->type != NUMBER || (tmp0->type == NUMBER && NTYPE(tmp0) == COMPLEX))
		return makeerror(1,0,"floor's r argument must be bound to a Number in the set or REALs");
	switch(NTYPE(tmp0))
	{
		case INTEGER:
			return tmp0;
		case REAL:
			return makereal(floor(AREAL(tmp0)));
		case RATIONAL:
			return makeinteger(0);
	}
}
SExp *
ftrunc(SExp *tmp0)
{
	double iptr = 0.0, val = 0.0, tmp = 0.0;
	if(tmp0->type != NUMBER)
		return makeerror(1,0,"inexact->exact's r argument must be bound to a Number in the set or REALs");
	if(NTYPE(tmp0) == REAL)
	{
		val = modf(AREAL(tmp0), &iptr);
		if(val < 0.5)
			return makeinteger((int)floor(AREAL(tmp0)));
		else
			return makeinteger((int)ceil(AREAL(tmp0)));
	}
	else if(NTYPE(tmp0) == RATIONAL)
	{
		tmp = (NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0);
		val = modf(tmp,&iptr);
		if(val < 0.5)
			return makeinteger((int)floor(tmp));
		else
			return makeinteger((int)ceil(tmp));	
	}
	else if(NTYPE(tmp0) == COMPLEX)
			return makeinteger(0);
	return tmp0;
}
SExp *fround(SExp *tmp0)
{
	double iptr = 0.0, val = 0.0, tmp = 0.0;
	if(tmp0->type != NUMBER)
		return makeerror(1,0,"inexact->exact's r argument must be bound to a Number in the set or REALs");
	if(NTYPE(tmp0) == REAL)
	{
		val = modf(AREAL(tmp0), &iptr);
		if(val < 0.5)
			return makereal(floor(AREAL(tmp0)));
		else
			return makereal(ceil(AREAL(tmp0)));
	}
	else if(NTYPE(tmp0) == RATIONAL)
	{
		tmp = (NUM(tmp0) * 1.0) / (DEN(tmp0) * 1.0);
		val = modf(tmp,&iptr);
		if(val < 0.5)
			return makereal(floor(tmp));
		else
			return makereal(ceil(tmp));	
	}
	else if(NTYPE(tmp0) == COMPLEX)
			return makereal(0);
	return tmp0;
}
SExp *
fin2ex(SExp *tmp0)
{
	double iptr = 0.0, val = 0.0;
	if(tmp0->type != NUMBER)
		return makeerror(1,0,"inexact->exact's r argument must be bound to a Number in the set or REALs");
	if(NTYPE(tmp0) == REAL)
	{
		val = modf(AREAL(tmp0), &iptr);
		if(val < 0.5)
			return makeinteger((int)floor(AREAL(tmp0)));
		else
			return makeinteger((int)ceil(AREAL(tmp0)));
	}
	else if(NTYPE(tmp0) == COMPLEX)
			return makeinteger(0);
	return tmp0;
}
SExp *
fexactp(SExp *tmp0)
{
	if(tmp0->type != NUMBER)
		return sfalse;
	if(tmp0->object.n->type == INTEGER || tmp0->object.n->type == RATIONAL)
		return strue;
	return sfalse;
}
SExp *
finexactp(SExp *tmp0)
{
	if(tmp0->type != NUMBER)
		return sfalse;
	if(tmp0->object.n->type == REAL || tmp0->object.n->type == COMPLEX)
		return strue;
	return sfalse;
}
SExp *
frealp(SExp *tmp)
{
	if(tmp->type != NUMBER)
		return sfalse;
	if(NTYPE(tmp) == REAL || NTYPE(tmp) == INTEGER || NTYPE(tmp) == RATIONAL)
		return strue;
	return sfalse;
}
SExp *
fcomplexp(SExp *tmp)
{
	if(tmp->type != NUMBER)
		return sfalse;
	if(NTYPE(tmp) == COMPLEX || NTYPE(tmp) == REAL || NTYPE(tmp) == INTEGER || NTYPE(tmp) == RATIONAL)
		return strue;
	return sfalse;
}
SExp *
frationalp(SExp *tmp)
{
	if(tmp->type != NUMBER)
		return sfalse;
	if(NTYPE(tmp) == INTEGER || NTYPE(tmp) == RATIONAL)
		return strue;
	return sfalse;
}
SExp *
fintegerp(SExp *tmp)
{
	if(tmp->type != NUMBER)
		return sfalse;
	if(NTYPE(tmp) == INTEGER)
		return strue;
	return sfalse;
}
/* syntactic functions */
SExp *
fbaselet(SExp *rst, Symbol *env)
{
	/* base-let form; this is similar to clojure's let form. In Scheme terms, it is most similar to let*,
	 * albeit with fewer parentheses. All let forms can be described as applications of lambdas, so there
	 * is no real point in including any forms, save for efficiency reasons. base-let is here becuase 
	 * of efficiency, and all other let forms may be defined atop of either this or lambdas. It is probably
	 * best if let was lambda-based, and let* was base-let based, but it's not of very much concern...
	*/
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
	Symbol *tenv = nil;
	int itmp = 0;
	itmp = pairlength(rst);
	if(itmp <= 1)
		return makeerror(1,0,"base-let (bindings :type :alist) (body :type :thunk) => S-EXPRESSION");
	tmp0 = car(rst); /* should be a-list bindings... */
	if(tmp0->type != PAIR)
		return makeerror(1,0,"bindings must be of type PAIR (and be an ALIST)");
	new_window(env);
	while(tmp0 != snil)
	{
		tmp1 = car(tmp0);
		if(tmp1->type != ATOM)
			return makeerror(1,0,"bindings must be an atom-indexed ALIST");
		tmp2 = car(cdr(tmp0));
		if(tmp2->type == PAIR)
			tmp2 = lleval(tmp2,tenv);
		else if(tmp2->type == ATOM)
			tmp2 = symlookup(tmp2->object.str,tenv);
		tenv = add_env(env,tmp1->object.str,tmp2);
		tmp0 = cdr(cdr(tmp0));
	}
	rst = cdr(rst);
	while(rst != snil)
	{
		if(mcar(rst)->type == PAIR)
			tmp0 = lleval(mcar(rst),tenv);
		else if(mcar(rst)->type == ATOM)
			tmp0 = symlookup(mcar(rst)->object.str,tenv);
		rst = cdr(rst);
	}
	return tmp0;
}
SExp *
fdotimes(SExp *src, Symbol *env)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil, *tmp3 = nil, *rst = src;
	Symbol *tenv = env;
	int itmp = 0;
	if(pairlength(rst) < 1)
		return makeerror(1,0,"d0times (var : ATOM, i : INTEGER) body => sexp");
	//~ printf("%s\n",typenames[mcar(rst)->type]);
	if(mcar(rst)->type != PAIR)
		return makeerror(1,0,"dot1mes (var : ATOM, i : INTEGER) body => sexp");
	if(mcar(mcar(rst))->type != ATOM)
		return makeerror(1,0,"dotime2 (var : ATOM, i : INTEGER) body => sexp");
	if(mcar(mcdr(mcar(rst)))->type == NUMBER && NTYPE(mcar(mcdr(mcar(rst)))) == INTEGER)
		tmp3 = mcar(mcdr(mcar(rst)));
	else if	(mcar(mcdr(mcar(rst)))->type == ATOM)
	{
		tmp3 = mcar(mcdr(mcar(rst)));
		tmp3 = symlookup(tmp3->object.str,env);
		if(tmp3->type != NUMBER || (tmp3->type == NUMBER && NTYPE(tmp3) != INTEGER))
			return makeerror(1,0,"dotim3s (var : ATOM, i : INTEGER) body => sexp");
	}
	else if	(mcar(mcdr(mcar(rst)))->type == PAIR)
	{
		tmp3 = mcar(mcdr(mcar(rst)));
		tmp3 = lleval(tmp3,env);
		if(tmp3->type != NUMBER || (tmp3->type == NUMBER && NTYPE(tmp3) != INTEGER))
			return makeerror(1,0,"dotim4s (var : ATOM, i : INTEGER) body => sexp");
	}
	else
		return makeerror(1,0,"dotim35 (var : ATOM, i : INTEGER) body => sexp");
	if(pairlength(mcar(rst)) == 3)
		tmp2 = mcar(mcdr(mcdr(mcar(rst)))); /* exit condition */
	else
		tmp2 = mcar(mcdr(mcar(rst)));
	tmp0 = mcar(mcar(rst));
	tmp1 = makenumber(INTEGER);
	//tenv = closure_add_env(tenv,env,tmp0->object.str,tmp1);
	new_window(env);
	rst = cdr(rst);
	while(tmp1->object.n->nobject.z < tmp3->object.n->nobject.z)
	{
		while(rst != snil)
		{
			tmp0 = car(rst);
			tmp0 = lleval(tmp0,tenv);
			if(tmp0->type == ERROR)
				return tmp0;
			rst = cdr(rst);
		}
		rst = cdr(src);
		tmp1->object.n->nobject.z++;
	}							
	if(tmp2->type == ATOM)
		return symlookup(tmp2->object.str,tenv);
	else if(tmp2->type == PAIR)
		return lleval(tmp2,tenv);
	return tmp2;
}
SExp *
fdocol(SExp *rst, Symbol *env)
{
	/*SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil, *tmp3 = nil, *tmp4 = nil;
	Symbol *tenv = env;
	int itmp = 0, iter = 0;
	if(pairlength(rst) < 1)
		return makeerror(1,0,"do-collection (var : ATOM c : COLLECTION [r : RETURN]) body => r");
	if(mcar(rst)->type != PAIR)
		return makeerror(1,0,"do-collection's cadr element must be a pair!");
	if(mcar(mcar(rst))->type != ATOM)
		return makeerror(1,0,"do-collection's caadr element must be an atom!");
	tmp0 = mcar(mcar(rst));
	tmp1 = mcar(mcdr(mcar(rst)));
	if(tmp1->type == PAIR)
		tmp1 = lleval(tmp1,env);
	else if(tmp1->type == ATOM)
		tmp1 = symlookup(tmp1->object.str,tenv);
		
	if(tmp1->type != STRING && tmp1->type != VECTOR && tmp1->type != PAIR)
		return makeerror(1,0,"do-collection's caddr element must be a collection!")
	if(tmp1->type == STRING)
	{
		if(tmp1->length <= 0)
			goto docol_ret;
		tmp2 = makechar(tmp1->object.str[0]);
		tenv = closure_add_env(tenv,env,tmp0->object.str,tmp2);
		for(iter = 1;iter <= tmp1->length;iter++)
		{
			tmp0 = cdr(rst);
			while(tmp0 != snil)
			{
				tmp3 = lleval(car(tmp0),tenv);
				tmp0 = cdr(tmp0);
			}
			tmp2->object.c = tmp1->object.str[iter];
		}
	}
	else if(tmp1->type == VECTOR)
	{
		if(tmp1->length <= 0)
			goto docol_ret;
		for(iter = 0;iter < tmp1->length;iter++)
		{
			tmp2 = tmp1->object.vec[iter];
			tenv = closure_add_env(tenv,env,tmp0->object.str,tmp2);
			tmp3 = cdr(rst);
			while(tmp3 != snil)
			{
				tmp4 = lleval(car(tmp3),tenv);
				tmp3 = cdr(tmp3);
			}
		}
	}
	else if(tmp1->type == PAIR)
	{
		itmp = pairlength(tmp1);
		if(itmp <= 0)
			goto docol_ret;
		while(tmp1 != snil)
		{
			tmp2 = car(tmp1);
			tenv = closure_add_env(tenv, env,tmp0->object.str,tmp2);
			tmp3 = cdr(rst);
			while(tmp3 != snil)
			{
				tmp4 = lleval(car(tmp3),tenv);
				tmp3 = cdr(tmp3);
			}
			tmp1 = cdr(tmp1);
		}
	}
docol_ret:*/
	/* need to check if (third (first rst)) is something, and return that something... */
	/*if(car(cdr(cdr(car(rst)))) == snil)
		return strue;
	tmp2 = car(cdr(cdr(car(rst))));
	if(tmp2->type == PAIR)
		tmp2 = lleval(tmp2,tenv);
	if(tmp2->type == ATOM)
		tmp2 = symlookup(tmp2->object.str,tenv);
	return tmp2;*/
	// not even sure if we'll keep docol as an internal primitive
	return svoid;
}
SExp *
fset(SExp *tmp0, SExp *tmp1, Symbol *env)
{
	Symbol *tenv = env;
	int itmp = 0;
	Window *hd = nil;
	Trie *data = nil;
	SExp *d = snil;
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
		return snil;
	hd = env->data;
	while(hd != nil)
	{
		data = hd->env;
		d = trie_get(tmp0->object.str,data);
		if(d != nil)
		{
			trie_put(tmp0->object.str,tmp1,data);
			return svoid;
		}
		hd = hd->next;
	}
	return makeerror(1,0,"set!: unknown symbol used in set form");
}
SExp *
fdef(SExp *tmp0, SExp *tmp1, Symbol *env)
{
	SExp *d = nil;
	if(tmp0->type != ATOM)
		return makeerror(1,0,"type clash: define's first argument must be an atom...");
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
	return svoid;
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
	return svoid;
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
	return svoid;
}
SExp *
ffn(SExp *rst, Symbol *env)
{
	SExp *tmp0 = nil, *tmp1 = nil, *tmp2 = nil;
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
	if(tmp1->type != PAIR && tmp1->type != NIL)
		return makeerror(1,0,"bindings must be either a PAIR, NIL");
	if(tmp1->type == PAIR)
	{
		/* let's check args, to test for what was mentioned above... */
		while(tmp1 != snil)
		{
			tmp2 = car(tmp1);
			if(tmp2->type != PAIR && tmp2->type != ATOM && tmp2->type != KEY)
				return makeerror(1,0,"formals must be either a pair or an atom");
			if(tmp2->type == PAIR)
			{
				if(mcar(tmp2)->type == KEY && mcar(mcdr(tmp1))->type == ATOM)
					return makeerror(1,0,"a positional argument may not come after a keyword argument");
			}
			else if(tmp2->type == KEY)
				if(strcasecmp("rest",tmp2->object.str) && strcasecmp("body",tmp2->object.str) && strcasecmp("opt",tmp2->object.str))
					return makeerror(1,0,"the only keyword objects for function definition are opt, rest & body");
			tmp1 = cdr(tmp1);
		}
	}
	//tmp0->object.closure.env = (void *)shallow_clone_env(env);
	tmp0->object.closure.env = (void *)env->data;
	tmp0->object.closure.params = car(rst);
	tmp2 = car(cdr(rst));
	if(tmp2->type == STRING)
	{
		//tmp0->object.closure.docstr = tmp2;
		trie_put(tmp0->metadata,"docstring",tmp2);
		tmp0->object.closure.data = cdr(cdr(rst));
	}
	else
		tmp0->object.closure.data = cdr(rst);
	return tmp0;
}
SExp *
fmeta(SExp *o)
{
	SExp *ret = nil, *tmp = nil;
	if(o == nil || o == snil)
		return makeerror(1,0,"meta o : SEXP [k : (STRING|KEYBOJ|ATOM)] => SEXP"); 
	switch(pairlength(o))
	{
		case 1:
			tmp = car(o);
			if(tmp == snil)
				return snil;
			else if(tmp->metadata == nil)
				return snil;
			ret = (SExp *)hmalloc(sizeof(SExp));
			ret->type = DICT;
			ret->object.dict = tmp->metadata;
			return ret;
		case 2:
			tmp = car(o);
			ret = car(cdr(o));
			if(tmp == snil)
				return snil;
			else if(tmp->metadata == nil)
				return snil;
			if(ret == snil)
				return snil;
			return trie_get(tmp->metadata,ret->objec.str);
	}
	return makeerror(1,0,"meta o : SEXP [k : (STRING|KEYBOJ|ATOM)] => SEXP"); 
}
SExp *
cloneenv(SExp *e)
{
	return snil;
}
#ifdef NEED_LOG2
double
log2(double x)
{
	return log(x) / log(2);
}
#endif
