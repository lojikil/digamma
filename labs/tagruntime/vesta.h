/* @(#) Internal definitions for Vesta
 */
#ifndef __VESTA_H
#define __VESTA_H

#define nil NULL
#define nul '\0'
#define MAX_STRING 2048

/* high-level information */
#define RELEASE "6.6-eta"

/* accessor macros */
#define mcar(x) (x)->object.clist.first
#define mcdr(x) (x)->object.clist.rest
#define mnconc(x) cons((x),snil)
#define set_int(x,y) (x)->object.n->nobject.z = (y)
#define get_int(x) (x)->object.n->nobject.z
#define set_str(x,y) (x)->object.str = hstrdup(y)
#define AINT(x) (x)->object.n->nobject.z
#define AREAL(x) (x)->object.n->nobject.real
#define NUM(x) (x)->object.n->nobject.rational.num
#define DEN(x) (x)->object.n->nobject.rational.den
#define CEREAL(x) (x)->object.n->nobject.complex.r
#define IMMAG(x) (x)->object.n->nobject.complex.i
#define TYPE(x) (x)->type
#define NTYPE(x) (x)->object.n->type
#define PORT(x) (x)->object.p
#define PTYPE(x) (x)->object.p->type
#define FILEPORT(x) (x)->object.p->pobject.f
#define FILEADDRESS(x) (x)->object.p->fileaddress
#define PROTONUMBER(x) (x)->object.p->protocol_number
#define NETBIND(x) (x)->object.p->bind
#define FILEMODE(x) (x)->object.p->mode
#define SOCKINFO(x) (x)->object.p->sock_info

#define LINE_DEBUG printf("Made it to %d in %s\n", __LINE__, __FUNCTION__)
/* def macros */
#define INTERNDEF(x) SExp *x(SExp *, Symbol *)

#define isnumeric(x) ((x >= '0' && x <= '9') || x == '+' || x == '-' || x == '.' || x == 'i' || x == '/')
#define issymdelim(x) ((x) != '"' && (x) != '\'' && (x) != '(' && (x) != ')' && (x) != '[' && (x) != ']' && (x) != '{' && (x) != '}' && (x) != ' ' && (x) != '\n' && (x) != '\t')

/* returns from lex, one or two per type */
#define TOK_LPAREN 0 /* #\( */
#define TOK_RPAREN 1 /* #\) */
#define TOK_LSQUAR 2 /* #\[ */
#define TOK_RSQUAR 3 /* #\] */
#define TOK_LCURLY 4 /* #\{ */
#define TOK_RCURLY 5 /* #\} */
#define TOK_INT    6 
#define TOK_LITSTR 7 /* "literal string" */
#define TOK_NQUOTE 8 /* quote #\' */
#define TOK_MQUOTE 9 /* meta-quote #\` */
#define TOK_UNQUOT 10 /* unquote #\, */
#define TOK_SPLICE 11 /* unquote-splicing (#\, #\@) */
#define TOK_SYMBOL 12
#define TOK_CHAR   13
#define TOK_TRUE   14
#define TOK_FALSE  15
#define TOK_SUCC   16
#define TOK_UNSUCC 17
#define TOK_NAME   18 /* named characters, i.e. #|space */
#define TOK_REAL   19
#define TOK_RATIO  20
#define TOK_COMPL  21
#define TOK_KEY    22 /* :key-object */
#define TOK_HEX    23 /* #xFF */
#define TOK_OCT    24 /* #o77 */
#define TOK_BIN    25 /* #b10 */
#define TOK_LEOF   26 /* #e */
#define TOK_LVOID  27 /* #v */
#define TOK_SERROR 97 /* EOF before end of string */
#define TOK_HERROR 98 /* syntax error on hash object */
#define TOK_EOF    99

/* llread states */
#define START_STATE 0
#define LIST_STATE 1
#define VECT_STATE 2
#define DICT_STATE 3
#define ERRO_STATE 99

#define DEBUG_PRI printf("Made it to %s %d\n", __FUNCTION__, __LINE__)

#define _read_return(x) if(sp >= 0) \
	{ \
		cons(x,cons(ret,snil));\
	} \
	else \
		return x;

/* for now; this should be something better... */
#define __return(x) __val = (x); \
		state = __INTERNAL_RETURN; \
		goto __base

#define lleval __seval
	 
typedef enum
{
 ATOM, NUMBER, CHAR, BOOL, GOAL, VECTOR, PAIR, STRING, PROCEDURE, 
 CLOSURE, FOREIGN, NIL, ERROR, PORT, MACRO, USER, TCONC, PRIM, DICT, 
 KEY, SYNTAX, SEOF, SVOID, ENVIRONMENT, USFOREIGN, CONTINUATION
} SExpType;

typedef enum
{
 INTEGER, REAL, RATIONAL, COMPLEX
} NumType;

typedef enum
{
	PSTRING, PNET, PFILE
} PortType;

typedef enum 
{
	OPCAR, OPCDR, OPCONS, OPLAMBDA, OPDEF, OPLENGTH, OPDEFMACRO, OPDEFSYN, OPQUOTE, OPPLUS,
	OPMULT, OPSUB, OPDIV, OPLIST, OPVECTOR, OPDICT, OPMKSTRING, OPMKVEC, OPMKDICT, OPEVAL, 
	OPAPPLY, OPSTRING, OPCCONS, OPFIRST, OPREST, OPCSET,OPCUPDATE, OPCSLICE, OPNTH, OPKEYS, OPPARTIAL,
	OPSET, OPGENSYM, OPAPPEND, OPEQ, OPTYPE, OPCALLCC, OPLT, OPGT, OPLTE, OPGTE, OPIF, OPUNWIND, 
	OPEXACT, OPINEXACT, OPCOMPLEX, OPREAL, OPINTEGER, OPRATIONAL, OPBEGIN, OPNUM, OPDEN,
	OPAND, OPOR, OPXOR, OPNEG, OPGCD, OPLCM, OPNUMEQ, OPMOD, OPQUOTIENT, OPREMAINDER, OPERROR, 
	OPMKPOL, OPMKRECT, OPIMAG, OPREALP, OPARG, OPMAG, OPSQRT, OPABS, OPEXP, OPLN, OPCONJBANG, OPPOLREC, OPRECPOL,
	OPSIN,OPCOS,OPTAN,OPACOS, OPASIN,OPATAN, OPATAN2, OPCOSH, OPSINH, OPTANH, OPEXP2, OPEXPM1, OPSHR, OPSHL,
	OPLOG2, OPLOG10, OPCONJ, OPSTRAP, OPASSQ, OPCOERCE, OPMKTCONC, OPTCONC, OPTCONCL, OPT2P, OPTCONCSPLICE,
	OPDEFREC, OPSETREC, OPSETCURIN, OPSETCUROUT, OPSETCURERR, OPCURIN, OPCUROUT, OPCURERR, OPMEMQ,
	OPQQUOTE, OPUNQUOTE, OPUNQSPLICE, OPDICHAS, OPEMPTY, OPCEIL, OPFLOOR, OPTRUNCATE, OPROUND,
	OPIN2EX, OPCLONENV, OPDEFENV, OPSETENV, OPSTDENV, OPFROMENV, OPMETA,OPRESET, OPSHIFT, OPCURTICK, 
	OPDEFAULTENV, OPNULLENV, __INTERNAL_RETURN, __PRE_APPLY, __POST_APPLY, __INT_CLOSURE, __PROC
} InternalOps;

typedef struct _NUM
{
	NumType type;
	union
	{
		int z;
		double real;
		struct
		{
			int num;
			int den;
		} rational;
		struct
		{
			double r;
			double i;
		} complex;
	} nobject;
} Number;

typedef struct
{
	/* fileaddress: port-filename
	 * mode: port-mode
	 * protocol_number: port-protocol-number
	 */
	char state; /* open, closed */
	PortType type;
	char *fileaddress; /* file name or address */
	int protocol_number; /* from struct protoent... */
	int bind; /* addres bound to... */
	char mode[3]; /* r,r+,w,w+,a,a+ */
	union
	{
		char *s;
		FILE *f;
		int fd; /* socket... need more than just this, no? */
	} pobject;
	void *sock_info; /* struct saddr_in pointer really... */
} Port;

/* Dictionaries in Vesta are backed by tries */
typedef struct __TRIE
{
	char key;
	int n_len;
	int n_cur;
	void *data; /* SExp */
	struct __TRIE **nodes;
} Trie ;

typedef struct _SEXP
{
	SExpType type;
	int length; /* length of aggregated s-expressions, such as strings or vectors */
	Trie *metadata;
	union
	{
		char c;
		char *str;
		struct _SEXP **vec;
		struct
		{
			int procnum;
			int arity;
			struct _SEXP *params;
			struct _SEXP *data;
			void *env; /* environment frame pointer */
		} closure;
		struct
		{
			struct _SEXP *first;
			struct _SEXP *rest;
		} clist;
		void *foreign;
		//struct _SEXP *(*procedure)(struct _SEXP *, void *); 
		void *procedure;
		Number *n;
		Port *p;
		struct
		{
			char source;
			int level;
			char *message;
		} error;
		struct
		{
			char evalp;
			int num;
			char *name;
		}primitive;
		Trie *dict;
	} object;
} SExp;


/* A hybrid trie/cons-list based approach... */
/* Eventually, this should be a vector w/ copying... */
typedef struct _WINDOW
{
	Trie *env;
	struct _WINDOW *next;
} Window;

typedef struct _SYM
{
	int cur_size;
	int cur_offset;
	int tick;
	Window *data;
	SExp *curstdin;
	SExp *curstdout;
	SExp *curstderr;
	SExp *snil;
	SExp *strue;
	SExp *sfalse;
	SExp *svoid;
	SExp *ssucc;
	SExp *sunsucc;
	SExp *fake_rsqr;
	SExp *fake_rcur;
	SExp *fake_rpar;
    /*
    SExp *qnan;
    SExp *snan;
    */
	SExp *seof;
} Symbol;

#endif
