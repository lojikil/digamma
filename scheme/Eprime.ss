; A restricted Digamma->C compiler that supports a subset of the Digamma spec.
; it's probably a crappy method of compilation, but it eases transition, since it
; reuses Vesta's runtime. The Best method would be to have a nice Type inference system
; that uses something similar to a tagged pointer for SExprs, rather than the heavy struct/union
; I use now. On top of that, I could unbox many things; this is definitely the tact I will take
; with future releases, but for now I'm just looking to speed up Digamma development & run times 
; :D
; zlib/png licensed (c) 2010 Stefan Edwards

(def *fnarit* {}) ; this is a dict of the various functions' arity
(def *fnmung* {}) ; maps the program's lambda's name to the munged version

(def string-join (fn (strs intersital)
	(def isj (fn (s i)
		(if (null? (cdr s))
			(cons (car s) '())
			(cons (car s) (cons i (isj (cdr s) i))))))
	(apply string-append (isj strs intersital))))
(def gen-number (fn (x)
        (cond
                (integer? x) (format "makeinteger(~n)" x)
		(rational? x) (format "makerational(~n,~n)" (numerator x) (denomenator x))
                (real? x) (format "makereal(~n)" x)
                (complex? x) (format "makecomplex(~n,~n)" (real-part x) (imag-part x))
                else (error "NaN"))))
(def gen-string (fn (x)
	(format "makestring(\"~s\")" x)))
(def gen-symbol (fn (x)
	(format "makeatom(\"~s\")" x)))
(def gen-vector (fn (x)
	(let ((n (length x)) (p (coerce x 'pair)))
		(string-append (format "vector(~n," n) (string-join (map gen-literal p) ",") ")"))))
(def gen-pair (fn (x)
	(let ((n (length x)))
		(string-append (format "list(~n," n) (string-join (map gen-literal x) ",") ")"))))
(def gen-bool (fn (x)
	       (if x
		"strue"
		"sfalse")))
(def gen-goal (fn (x)
	       (if (eq? x #s)
		"ssucc"
		"sunsucc")))
(def gen-literal (fn (x)
	(cond
		(number? x) (gen-number x)
		(string? x) (gen-string x)
		(vector? x) (gen-vector x)
		(pair? x) (gen-pair x) ; really, need to tell what type of code to generate here...
		(dict? x) (gen-dict x)
		(eq? x '()) "snil"
		(symbol? x) (gen-symbol x)
		(bool? x) (gen-bool x)
		(goal? x) (gen-goal x)
		else (error (format "unsupported data type for code generation: ~s" (type x))))))
		 
(def cmung-name (fn (s)
		 (display "Made it to cmung\n")
	(def imung (fn (s i thusfar)
		(cond
			(>= i (length s)) thusfar 
			(ascii-acceptable? (nth s i))  (imung s (+ i 1) (append thusfar (list (nth s i))))
			else (imung s (+ i 1) thusfar))))
	(display "Returning from cmung\n")
	(apply string (imung (coerce s 'string) 0 '()))))
(def ascii-acceptable? (fn (c)
	(or
		(and (char->=? c #\a) (char-<=? c #\z))
		(and (char->=? c #\A) (char-<=? c #\Z))
		(and (char->=? c #\0) (char-<=? c #\9))
		(eq? c #\_))))
(def check-tail-call (fn (proc)
	"walk through the code of proc, and check if it calls itself; return #t if:
	- a bottom if has a call in either it's <then> or <else> suite
	- a bottom begin has a self-call in the tail"
	#f))
(def lift-lambda (fn (name code)
	(let ((fixname (cmung-name name)))
	 (cset! *fnmung* name fixname)
	 (cset! *fnarit* name (length (car code)))
	 (format "SExp *~%~s(~s)\n{\n\tSExp *ret = nil;\n\t~s \n\treturn ret;\n}\n" fixname (string-join (map (fn (x) (format "SExp *~a" x)) (car code)) ",") (gen-begin (cdr code))))))
(def lift-tail-lambda (fn (name code)
	"lift-tail-lambda is for when check-tail-call returns #t; basically, this generates a while loop version of the same lambda"
	#f))
(def defined-lambda? (fn (name)
	(dict-has? *fnmung* name)))
(def call-lambda (fn (name args)
 (if (= (length args) (nth *fnarit* name))
  (format "~s(~s)" (nth *fnmung* name) (string-join (map (fn (x) (gen-code x)) args) ","))
  (error (format "incorrect arity for ~S~%" (coerce name 'string))))))

; need to change this:
;  - check if <then> or <else> is a (begin ...)
;  - if not, say ret = (gen-code ...)
;  - if so, do nothing (and make gen-begin set ret = final code...)
(def gen-if (fn (args)
	     (let ((<cond> (gen-code (car args)))
		   (<then> (if (eq? (caadr args) 'begin) (gen-code (cadr args)) (string-append "ret = " (gen-code (cadr args)) ";\n")))
		   (<else> (if (eq? (caaddr args) 'begin) (gen-code (caddr args)) (string-append "ret = " (gen-code (caddr args)) ";\n")))
		   (<it> (gensym 'it)))
	      (format "SExp *~s = ~s;~%
	       if(~s == nil || ~s->type == NIL || ((~s->type == BOOL || ~s->type == GOAL) && ~s->object.c))
	       {
	       	 ~s
		}
		else
		{
			~s
		}~%" <it> <cond> <it> <it> <it> <it> <it> <then> <else>))))
	      
(def gen-code (fn (x)
	(if (pair? x) 
		(cond
			(eq? (car x) 'def) 
				(if (not (symbol? (car (cdr x))))
					(error "def p : SYMBOL e : SEXPRESSION => VOID")
					(if (not (pair? (car (cdr (cdr x)))))
						(format "fdef(~s,~s);" (gen-literal (car (cdr x))) (gen-literal (car (cdr (cdr x)))))
						(if (not (eq? (car (car (cdr (cdr x)))) 'fn))
							(format "fdef(~s,~s);" (gen-literal (car (cdr x))) (gen-code (car (cdr (cdr x)))))
							(lift-lambda (car (cdr x)) (cdr (car (cdr (cdr x))))))))
			(eq? (car x) 'load) #t
			(eq? (car x) 'import) #t
			(eq? (car x) 'use) #t
			(eq? (car x) 'from) #t
			(eq? (car x) 'let) #t ; let should be a top-level form, rather than expand to lambda(s)
			(eq? (car x) 'with) #t ; same goes for with
			(eq? (car x) 'quote) (gen-literal (car (cdr x)))
			(eq? (car x) 'module) 'MODULE
			(eq? (car x) 'if) (gen-if (cdr x)) ; if & other primitive syntax needs to be handled here
			(eq? (car x) 'cond) (gen-cond (cdr x)) ; should be nearly identical to if, but with more else if's 
			(eq? (car x) 'begin) (gen-begin (cdr x))
			(eq? (car x) 'list) (format "list(~n,~s)" (length (cdr x)) (string-join (map gen-code (cdr x)) ","))
			(eq? (car x) 'vector) (format "vector(~n,~s)" (length (cdr x)) (string-join (map gen-code (cdr x)) ","))
			(pair? (car x)) #t
			(defined-lambda? (car x)) (call-lambda (car x) (cdr x))
			(primitive-form? (car x)) (gen-primitive x) 
			(primitive-proc? (car x)) (gen-prim-proc x) ;display & friends
			else 'EVAL-FORM)
		(if (symbol? x)
		 (coerce x 'string)
		 (gen-literal x)))))
; TODO:
; - make macro to "call" a specific sub-routine
; - gensym the base label
; - generate a C macro to jump to this "base label"
; - collate lambda "states" to an enum that can be placed in a header file for
; Awesome things to do:
; - call graphs
; - function instrumentation (for debugging)
; - static checks of availability
; - useful lambda lifting 
; - inclusion of types & typed-syntax expansion
; - c-lambdas, c-macros, c-syntax
(def header-out (fn (p n) 
		 "output C headers & any top-level structure to output file"
		 (def BASE (gensym 'BASE))
		 ; would be nice to store which of these headers is needed by which
		 ; primitives, and only include accordingly.
		 (foreach-proc (fn (x) (display x p) (newline p)) '("#include <stdio.h>"
"#include <stdlib.h>"
"#include <string.h>"
"#include <unistd.h>"
"#include <gc.h>"
"#include <math.h>"
"#include <sys/param.h>"
"#include <fcntl.h>"
"#include <sys/time.h>"
"#include <sys/types.h>"
"#include <sys/stat.h>"
"#include <sys/wait.h>"
"#include <sys/socket.h>"
"#include <netdb.h>"
"#include <netinet/in.h>"
"#include <arpa/inet.h>"
"#include <dirent.h>"
"#include <signal.h>"
"#include <errno.h>"
"#include <stdarg.h>"
"#include \"vesta.h\""
"#include \"eris-out.h\""))
		 (display (format "~%#define __return(x) __val = (x); state = __INT_ERIS_RETURN; goto ~a;~%~%" BASE) p)
		 (display (format "~%#define __call(x,y) state = (x); nextstate = (y);~%~%") p)
		 (display (format "void~%~s()~%{~%" n) p)))
(def footer-out (fn (p)
		 "finalize C code to output file"
		 (display "\t}\n}\n" p)))
(def dump-states (fn (p)
	(display "#ifndef __ERIS_OUT_H\n#define __ERIS_OUT_H\n" p)
	(display "typedef enum\n{\n" p)
	(display (string-join (map (fn (x) (format "~a" (nth *fnmung* x))) (keys *fnmung*)) ",\n") p) 
	(display "\n}\n#endif\n" p)))
(def eprime (fn ()
	   "Main code output"
	   (def inner-eris (fn (i o) 
		(with x (read i)
		 (if (eq? x #e)
			#v
		  (begin
		   (display (format "~s~%" (gen-code x)) o)
		   (inner-eris i o))))))
	   (let ((in (open (nth *command-line* 0) :read)) 
		 (out (open (nth *command-line* 1) :write))
		 (name (nth *command-line* 2))
		 (e-oh (open "eris-out.h" :write)))
	    (header-out out name)
	    (inner-eris in out)
	    (footer-out out)
	    (dump-states e-oh)
	    (close in)
	    (close e-oh)
	    (close out))))
; The basic system is this:
; - read form from file
; - call syntax-expand
; - call macro-expand
; - call gen-code on what's left
; - loop to the first item until #e
; Primitive handling functions for Eris
; also defines *primitives* a global dict of all internal forms
; TODO:
;  - make *primitives* result a vector: [arity syntax? internal-c-function]
;    + arity is the number of parameters to the C function [0 means just pass a list]
;    + syntax? means if this form should have it's arguments eval'd before applying it
;    + internal-c-function is the low-level C function that backs this primitive in Vesta's runtime
; zlib/png licensed Copyright 2010 Stefan Edwards 

(def *primitives* {
:car [1 #f "car"]
:cdr [1 #f "cdr"] 
:cons [2 #f "cons"] 
:length [1 #f "flength"] 
:+ [0 #f "fplus"]
:exact? [1 #f "fexactp"]
:inexact? [1 #f "finexactp"]
:real? [1 #f "frealp"]
:integer? [1 #f "fintegerp"]
:complex? [1 #f "fcomplexp"]
:rational? [1 #f "frationalp"]
:numerator [1 #f "fnum"]
:denomenator [1 #f "fden"]
:* [0 #f "fmult"]
:type [1 #f "ftype"]
:- [0 #f "fsubt"]
:/ [0 #f "fdivd"]
:gcd [0 #f "fgcd"]
:lcm [0 #f "flcm"]
:ceil [1 #f "fceil"]
:floor [1 #f "ffloor"]
:truncate [1 #f "ftruncate"]
:round [1 #f "fround"]
:inexact->exact [1 #f "fin2ex"]
:eq? [2 #f "eqp"]
:< [0 #f "flt"] 
:> [0 #f "fgt"] 
:<= [0 #f "flte"]
:>= [0 #f "fgte"]
:= [0 #f "fnumeq"]
:quotient [2 #f "fquotient"]
:modulo [2 #f "fmodulo"]
:remainder [2 #f "fremainder"]
;:set! #t
;:fn #t
:& [2 #f "fbitand"]
:| [2 #f "fbitor"]
:^ [2 #f "fbitxor"]
:~ [2 #f "fbitnot"]
:make-vector [0 #f "fmkvector"]
:make-string [0 #f "fmakestring"]
:append [0 #f "fappend"]
:first [0 #f "ffirst"]
:rest [0 #f "frest"]
:ccons [0 #f "fccons"] 
:nth [0 #f "fnth"]
:keys #t 
:partial-key? #t
:cset! [0 #f "fcset"]
:empty? #t
:gensym #t
:imag-part #t
:real-part #t
:make-rectangular #t
:make-polar #t
:magnitude #t
:argument #t
:conjugate! #t
:conjugate #t
:polar->rectangular #t
:rectangular->polar #t
:sin #t
:cos #t
:tan #t
:asin #t
:acos #t
:atan #t
:atan2 #t
:cosh #t
:sinh #t
:tanh #t
:exp #t
:ln #t
:abs #t
:sqrt #t
:exp2 #t
:expm1 #t
:log2 #t
:log10 #t
:<< #t
:>> #t
:begin #t
:string-append #t
:apply #t
:assq #t
:defrec #t
:set-rec! #t
:dict #t
:make-dict #t
:dict-has? #t
:coerce #t
:error #t
:cupdate #t
:cslice #t
:tconc! #t
:make-tconc #t
:tconc-list #t
:tconc->pair #t
:tconc-splice! #t
:if #t
:eval #t
:meta! #t
:current-tick #t
})
(def *prim-proc* {
 :display [0 "f_princ"]
 :read-char #t
 :write-char #t
 :read-buffer #t
 :write-buffer #t
 :read-string #t
 :write-string #t
 })
(def primitive-form? (fn (x)
	(dict-has? *primitives* x)))
(def gen-begin (fn (l)
		(if (eq? (cdr l) '())
		 (if (and (pair? (car l)) (eq? (car (car l)) 'if))
		  (string-append (gen-code (car l)) ";\n") 
		  (string-append "ret = " (gen-code (car l)) ";\n"))
		 (string-append (gen-code (car l)) ";\n" (gen-begin (cdr l))))))
(def gen-primitive (fn (x)
	(let ((f (nth *primitives* (car x))) (args (cdr x)))
	 (if (= (nth f 0) 0) ; arity
	  (format "~s(list(~n,~s))" (nth f 2) (length args) (string-join (map gen-code args) ","))
	  (if (= (length args) (nth f 0))
	   (format "~s(~s)" (nth f 2) (string-join (map gen-code args) ","))
	   (error (format "eris: incorrect number of arguments to ~s" (nth f 2))))))))
(def primitive-proc? (fn (x)
	(dict-has? *prim-proc* x)))
(def gen-prim-proc (fn (f)
#f))
