; basically, allow scheme-style (define (f x) somebody...)
; should probably change primitives here: make 'def the primitive
; and 'define the macro, for normal Schemer purposes...
(define-macro define (head :rest body)
	(cond 
        ((eq? (type head) "Pair") (list 'def (car head) (append (list 'fn (cdr head)) body)))
        ((eq? (type head) "Symbol") (list 'def head (car body)))))
(define princ display)
; normal scheme predicates...
(define (null? x) (eq? (type x) "Nil"))
(define (pair? x) (eq? (type x) "Pair"))
(define (string? x) (eq? (type x) "String"))
(define (char? x) (eq? (type x) "Character"))
(define (vector? x) (eq? (type x) "Vector"))
(define (symbol? x) (eq? (type x) "Symbol"))
(define (number? x) (eq? (type x) "Number"))
(define (port? x) (eq? (type x) "Port"))
(define (procedure? x) 
	(or
		(eq? (type x) "Closure")
		(eq? (type x) "Primitive")
		(eq? (type x) "Procedure")))
(define (key? x) (eq? (type x) "Key"))
(define (syntax? x) (eq? (type x) "Syntax"))
(define (macro? x) (eq? (type x) "Macro"))
(define atom? symbol?) ;; bad habbit ala atomp
(define lambda fn) ;; for schemers

; #e should never be used by it self, since 
; I may change the syntax to conform more to
; R6RS than the misc. R5RS syntaxes that include #!eof.
; for now though, #e *is* specified in the Digamma 
; base spec.
(define (eof-object? n) (if (eq? n #e) #t))
; Aggregating predicates:
(define (collection? x) (or (pair? x) (vector? x) (string? x)))
(define (atomic? x) (or (char? x) (number? x) (procedure? x) (key? x) (symbol? x)))

; stupid helper functions
(define (ncons x) (cons x '()))
(define (endswith source end) ; test if 'source ends with 'end
        (base-let (ls (length source)
                le (length end))
                (if (>= ls le)
                        (eq? (source (- ls le) ls) end))))
(define (beginswith source begin)
	(base-let (ls (length source) 
				lb (length begin))
			(if (>= ls lb)
				(eq? (source 0 lb) begin))))
(define negative? (fn (x)
	(cond
		((number? x) (< x 0)))))
(define positive? (fn (x)
	(cond
		((number? x) (> x 0)))))

; useful syntax definition
(define-syntax if  
        ((if <cond> <then>)     
                (cond (<cond> <then>)))
        ((if <cond> <then> <else>)
                (cond (<cond> <then>) (else <else>))))
;(define-macro if (<test> <then> :opt <else>) ; <else> should be :opt
;	(cond
;		((null? <else>)
;			(list 'cond (list <test> <then>)))
;		(else
;			(list 'cond (list <test> <then>) (list 'else <else>)))))

(define-macro and (<item> :rest <body>) 
	(cond 
		((null? <body>) <item>)
		(else (list 'if <item> (cons 'and <body>)))))
(define-macro or (<item> :rest <body>) 
	(cond 
		((null? <body>) <item>)
		(else (list 'if <item> #t (cons 'or <body>)))))
;useful constants
(define *dirs* '()) ; pushd/popd work out of here...
(define *pi* 3.1459265359)
(define *e* (exp 1))
(define *site-lib* '("." "./lib" "~/.digamma/lib")) ; for use in require
(define S_IFMT #o170000)
(define S_IFIFO #o010000)
(define S_IFCHR #o020000)
(define S_IFDIR #o040000)
(define S_IFBLK #o060000)
(define S_IFREG #o100000)
(define S_IFLNK #o120000)
(define S_IFSOCK #o140000)
(define S_IFWHT #o160000)
(define S_ISUID #o004000)
(define S_ISGID #o002000)
(define S_ISVTX #o001000)
(define S_IRUSR #o000400)
(define S_IWUSR #o000200)
(define S_IXUSR #o000100)

;; cNr car/cdr composition to 4 levels required by R5RS...
;; these should be rewritten to refelect better style...
(define caar (fn (x) (if (pair? x) (car (car x)) #f)))
(define cadr (fn (x) (if (pair? x) (car (cdr x)) #f)))
(define cdar (fn (x) (if (pair? x) (cdr (car x)) #f)))
(define cddr (fn (x) (if (pair? x) (cdr (cdr x)) #f)))
(define caaar (fn (x) (if (pair? x) (car (car (car x))) #f)))
(define caadr (fn (x) (if (pair? x) (car (car (cdr x))) #f)))
(define cadar (fn (x) (if (pair? x) (car (cdr (car x))) #f)))
(define caddr (fn (x) (if (pair? x) (car (cdr (cdr x))) #f)))
(define cdaar (fn (x) (if (pair? x) (cdr (car (car x))) #f)))
(define cdadr (fn (x) (if (pair? x) (cdr (car (cdr x))) #f)))
(define cddar (fn (x) (if (pair? x) (cdr (cdr (car x))) #f)))
(define cdddr (fn (x) (if (pair? x) (cdr (cdr (cdr x))) #f)))
(define caaaar (fn (x) (if (pair? x) (car (car (car (car x)))) #f)))
(define caaadr (fn (x) (if (pair? x) (car (car (car (cdr x)))) #f)))
(define caadar (fn (x) (if (pair? x) (car (car (cdr (car x)))) #f)))
(define caaddr (fn (x) (if (pair? x) (car (car (cdr (cdr x)))) #f)))
(define cadaar (fn (x) (if (pair? x) (car (cdr (car (car x)))) #f)))
(define cadadr (fn (x) (if (pair? x) (car (cdr (car (cdr x)))) #f)))
(define caddar (fn (x) (if (pair? x) (car (cdr (cdr (car x)))) #f)))
(define cadddr (fn (x) (if (pair? x) (car (cdr (cdr (cdr x)))) #f)))
(define cdaaar (fn (x) (if (pair? x) (cdr (car (car (car x)))) #f)))
(define cdaadr (fn (x) (if (pair? x) (cdr (car (car (cdr x)))) #f)))
(define cdadar (fn (x) (if (pair? x) (cdr (car (cdr (car x)))) #f)))
(define cdaddr (fn (x) (if (pair? x) (cdr (car (cdr (cdr x)))) #f)))
(define cddaar (fn (x) (if (pair? x) (cdr (cdr (car (car x)))) #f)))
(define cddadr (fn (x) (if (pair? x) (cdr (cdr (car (cdr x)))) #f)))
(define cdddar (fn (x) (if (pair? x) (cdr (cdr (cdr (car x)))) #f)))
(define cddddr (fn (x) (if (pair? x) (cdr (cdr (cdr (cdr x)))) #f)))

; Misc. functions
(define (unzip ll)
     (base-let (xl '() yl '())
        (while (not (null? ll))
                (set! xl (append xl (ncons (caar ll))))
                (set! yl (append yl (ncons (cadar ll))))
                (set! ll (cdr ll)))
        (list xl yl)))
(define (unzip-r ll)
	(define (inner-u-r xl yl sl)
		(if (null? sl)
			(list xl yl)
			(inner-u-r (append xl (ncons (caar sl))) (append yl (ncons (cadar sl))) (cdr sl))))
	(inner-u-r '() '() ll))
(define (zip xs ys) ;yuck iterative version
	(base-let (ll '())
		(while (not (null? xs))
			(set! ll (append ll (ncons (cons (car xs) (cons (car ys) '())))))
			(set! xs (cdr xs))
			(set! ys (cdr ys)))
		ll))
;; This is to replace the above "explicitly iterated" version
;; once TCO & environments are fully improved in Vesta...
(define (zip-r xs ys)
	(if (null? xs)
		'()
		(cons (cons (car xs) (cons (car ys) '())) (zip-r (cdr xs) (cdr ys)))))
(define (map proc lst)
	(if (empty? lst)
		'()
		(cons (proc (car lst)) (map proc (cdr lst)))))
(define (foldl proc val lst)
	(cond
		((empty? lst) val)
		(else (proc (foldl proc val (rest lst)) (first lst))))) 
(define (foldr proc val lst)
	(cond
		((empty? lst) val) ; when this was null?, caused a SEGFAULT with illegal mem access. Check into this (must be first or rest...)
		(else (proc (first lst) (foldr proc val (rest lst))))))
(define (filtre proc coll) ; generic now, for map-able collections (string, vector, list)
	(cond
		((empty? coll) coll)
		((proc (first coll)) (ccons (first coll) (filtre proc (rest coll))))
		(else (filtre proc (rest coll)))))
(define filter filtre)
(define (exact->inexact x) (* x 1.0))
; Some logical *functions* that should be replaces with macros...
; useful for composition though...
(define (not x)
	(cond
		((eq? x #s) #u)
		((eq? x #u) #s)
		((eq? x #f) #t)
		((eq? x '()) #t)
		(else #f)))
(define (logical-bor x y) 
	(cond 
		((eq? x #t) #t) 
		(else y)))
(define (logical-band x y) 
	(cond 
		((eq? x #t) y)))
(define (logical-bxor x y) 
	(logical-band (logical-bor x y) (not (logical-band x y))))

; bit-wise functions
(define (cnt-one x)
	(base-let (z x count 0)
		(while (not (= 0 z))
			(cond
				((= (& z #x1) 1) (set! count (+ 1 count))))
			(set! z (>> z 1)))
		count))
(define (integer->bitstring n)
	(define (ii->b in offset)
		(if (= offset 32)
			'()
			(cons (if (= (& in #x80000000) #x80000000) #\1 #\0) (ii->b (<< in 1) (+ offset 1)))))
	(if (integer? n)
		(ii->b n 0)))
;; Misc shell features...
; pushd does the obvious: it pushes the current directory & chdirs elsewhere...
(define (pushd dir)
	(if (-d dir)
		(begin
			(set! *dirs* (cons (pwd) *dirs*))
			(chdir dir))))
; popd returns to the previous location...
(define (popd)
	(if (not (null? *dirs*))
		(begin
			(base-let (dest (car *dirs*))
				(set! *dirs* (cdr *dirs*))
				(chdir dest)))))
				
; test(1)-style long hand functions...
(define (directory? x) 
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFDIR)
			#f)))
(define (file? x)
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFREG)
			#f)))
(define (exist? x) (if (eq? (sysstat x) #f) #f #t))
(define (character-device? x) 
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFCHR)
			#f)))
(define (block-device? x)
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFBLK)
			#f)))
(define (fifo-device? x)
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFIFO)
			#f)))
(define (socket-device? x)
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFSOCK)
			#f)))
(define (symlink? x)
	(base-let (val (sysstat x))
		(if (vector? val)
			(eq? (& (vector-ref val 2) S_IFMT) S_IFLNK)
			#f)))

; test(1)-style shorthand "aliases"
(define -d directory?)
(define -f file?)
(define -e exist?)
(define -c character-device?)
(define -b block-device?)
(define -S socket-device?)
(define -L symlink?)

; Misc. functions
; once TCO is implemented, these should all be rewritten in an
; idiomatic way, using recursion...
(define (vector->string v)
	(if (vector? v)
		(base-let (str (make-string (length v)))
			(dotimes (idx (length v) str)
				(string-set! str idx (cond ((integer? (v idx)) (int->char (v idx))) ((char? (v idx)) (v idx)) (else #\space)))))))
(define (string->vector s)
	(if (string? s)
		(base-let (vec (make-vector (length s) :no-init))
			(dotimes (idx (length s) vec)
				(vector-set! vec idx (s idx))))))
(define (collection->vector l) (if (collection? l) (foldr ccons [] l))) ; not the most efficient, but conceptually the simplest
(define (collection->list l) (if (collection? l) (foldr ccons '() l)))
(define (list->vector l) 
	(if (pair? l)
		(apply vector l)
		(collection->vector l)))
; More Scheme

; Pretty inefficient way to do equal?, since rest returns an entirely new
; vector for each call (same for strings)...
; still, for now, quite simple. The responsible thing to do may be to
; move this into core...
(define (equal? x y)
	(base-let (xt (type x) yt (type y))
		(cond
			((not (eq? xt yt)) #f)
			((eq? xt "Number") (= x y))
			((collection? x) (and (equal? (first x) (first y)) (equal? (rest x) (rest y))))
			(else (eq? x y)))))
(define (eqv? x y)
	(base-let (xt (type x) yt (type y))
		(cond
			((not (eq? xt yt)) #f)
			((eq? xt "Number") (= x y))
			(else (eq? x y)))))
(define-macro let (fst rst :rest body)
	(cond
		((pair? fst) 
			(base-let (both (unzip fst))
				(append (ncons (append (list 'fn (car both)) (ncons rst) body)) (car (cdr both)))))
		(else
			(base-let (both (unzip rst))
			(list 'base-let (list fst '()) (list 'set! fst (append (list 'fn (car both)) body)) (append (ncons fst) (car (cdr both))))))))

(define (sub-map-vector src dst idx proc)
	(cond
		((= idx (length dst)) dst)
		(else
			(begin	
				(vector-set! dst idx (proc (src idx)))
				(sub-map-vector src dst (+ idx 1) proc)))))
(define (map-vector p v)
	(sub-map-vector v (make-vector (length v) :no-init) 0 p))
(define (map-vector! p v)
	(sub-map-vector v v 0 p))
(define (foreach p c)
	(define (sub-foreach c p idx)
		(if (<= idx (length c))
			(begin
				(p (nth c idx))
				(sub-foreach c p (+ idx 1)))))
	(sub-foreach c p 0))
(define (listify lst end)
                (cond
                        ((null? lst) (ncons end))
                        ((null? (cdr lst)) (cons (car lst) (ncons end)))
                        (else (cons (car lst) (ncons (listify (cdr lst) end))))))		
; Arc-like compose
(define-macro compose (:rest x)
	(list 'fn '(y)	(listify x 'y)))
; not really correct, as '-> doesn't return a lambda...
(define -> compose)

;; Maths
;; This function should have a few sanity checks:
;; 0 - if b == 0 return 0
;; 1 - if b == 1 return 1
;; 2 - if b == 2 return (exp2 b)
;; 3 - if x subsetmember(Z) (i.e. (integer? x)), iterative power
;; 4 - (and (complex? x) (complex? b)), special complex rules...
;; 5 - default to e ^ (x * ln(b))
;; check 3 should actually be (and (integer? x) (positive? x))...

(define (expt b x) 
	(cond
		((= b 0) 0)
		((= b 1) 1)
		((= b 2) (exp2 x))
		;((integer? x) (ipow b x))
		;((and (complex? b) (complex? x)) stuff)
		(else (exp (* x (ln b))))))

(define (positive? x)
	(and (number? x) (> x 0)))
(define (negative? x)
	(and (number? x) (< x 0)))
(define (zero? x)
	(and (number? x) (= x 0)))
(define (odd? x) (and (number? x) (= (modulo x 2) 1)))
(define (even? x) (and (number? x) (= (modulo x 2) 0)))

;; Scheme I/O primitives
(define (open-input-file fil) (open fil :read))
(define (open-output-file fil) (open fil :write))
(define (close-input-port p) (close p))
(define (close-output-port p) (close p))
(define (call-with-input-file file proc)
	(base-let (fd (open file :read) ret (proc fd))
		(close fd)
		ret))
(define (call-with-output-file file proc)
	(base-let (fd (open file :write) ret (proc fd))
		(close fd)
		ret))
		
;; Potentially useful things..
;; begin{0-2} match CL's prog1-3
;; we probably don't want to evaluate these out of order...
;; the let form should be moved *after* the list of the previous
;; forms (f0->f1) so that we still evaluate this in order...
(define-macro begin0 (f0 :rest body)
	(base-let (<value> (gensym))
		(append (list 'base-let (list <value> f0)) body (list <value>))))

(define-macro begin1 (f0 f1 :rest body)
	(base-let (<value> (gensym))
		(append (list 'begin) (list f0) (ncons (append (list 'base-let (list <value> f1)) body (list <value>))))))
(define-macro begin2 (f0 f1 f2 :rest body)
	(base-let (<value> (gensym))
		(append (list 'begin) (list f0 f1) (ncons (append (list 'base-let (list <value> f2)) body (list <value>))))))
(define (list-join lst val)
	(cond
		((null? lst) '())
		((pair? lst) (cons (car lst) (cons val (list-join (cdr lst) val))))
		(else #f)))
;; Wrappers around set*id
(define (setuid n) (set*id :uid n))
(define (seteuid n) (set*id :euid n))
(define (setgid n) (set*id :gid n))
(define (setegid n) (set*id :egid n))

; R5RS character predicates...
(define (char<? a b)
	(< (char->int a) (char->int b)))
(define (char<=? a b)
	(<= (char->int a) (char->int b)))
(define (char>? a b)
	(> (char->int a) (char->int b)))
(define (char>=? a b)
	(>= (char->int a) (char->int b)))
(define (char=? a b)
	(= (char->int a) (char->int b)))
(define (char-alphabetic? a)
	(and (char-ci>=? a #\a) (char-ci<=? a #\z)))
(define (char-downcase a)
	(if (and (char>=? a #\A) (char<=? a #\Z))
		(int->char (+ (char->int a) 32))
		a))
(define (char-upcase a)
	(if (and (char>=? a #\a) (char<=? a #\z))
		(int->char (- (char->int a) 32))
		a))
(define (char-lower-case? a)
	(and (char>=? a #\a) (char<=? a #\z)))
(define (char-upper-case? a)
	(and (char>=? a #\A) (char<=? a #\Z)))
(define (char-numeric? a)
	(and (char>=? a #\0) (char<=? a #\9)))
(define (char-whitespace? a)
	(or (char=? a #\space) (char=? a #\linefeed) (char=? a #\tab) (char=? a #\carriage)))
(define (char-ci<? a b)
	(< (coerce (char-downcase a) :int) (coerce (char-downcase b) :int)))
(define (char-ci<=? a b)
	(<= (coerce (char-downcase a) :int) (coerce (char-downcase b) :int)))
(define (char-ci>? a b)
	(> (coerce (char-downcase a) :int) (coerce (char-downcase b) :int)))
(define (char-ci>=? a b)
	(>= (coerce (char-downcase a) :int) (coerce (char-downcase b) :int)))
(define (char-ci=? a b)
	(= (coerce (char-downcase a) :int) (coerce (char-downcase b) :int)))

;; Library functions
(define (import file)
	#f)
(define (from file imports) #f)
(define (tilde-expand st)
	(base-let (home (sys-getenv "HOME") l (length st))
		(if (string? home)
			(string-append home (st 1 l)))))
(define (find-requirement lib-path lib) ; should make an option to append an ending if one is missing,
	(cond							; so that we can test for each of '(".ss" ".scm" ".sls")...
		((null? lib-path) #f)
		((and (string? (car lib-path)) (beginswith (car lib-path) "~") (-f (string-append (tilde-expand (car lib-path)) "/" lib))) 
			(string-append (tilde-expand (car lib-path)) "/" lib))
		((-f (string-append (car lib-path) "/" lib)) (string-append (car lib-path) "/" lib)) 
		(else (find-requirement (cdr lib-path) lib))))
; this will have to be fixed once the environment patches are in, as this currently
; relies upon the fact that 'define bubbles to the bottom to add symbols...
(define (require file)
	(if (or (endswith file ".ss") (endswith file ".scm") (endswith file ".sls"))
		(load (find-requirement *site-lib* file)) ; should check for #f...
		(load (find-requirement *site-lib* (string-append file ".ss")))))
(define (require-as file) #f)

(define (list->string lst) ; should really be (apply string lst)
	(apply string lst))

; Simple destructive macros
; works on all collection types
(define-macro push! (<var> <obj>)
	(list 'set! <var> (list 'ccons <obj> <var>)))
(define-macro pop! (<var>)
	(list 'begin0 (list 'first <var>) (list 'set! <var> (list 'rest <var>))))
	
; Anaphoric macros from purgatory
; not really bad, per se
; just ugly
; ugsome
; whatever
(define-macro until-it (<it-bind> <it-pred> :body <thunk>)
	(list 'base-let (list 'it <it-bind>) (append (list 'while (list 'not <it-pred>)) <thunk> (ncons (list 'set! 'it <it-bind>)))))
(define-macro while-it (<it-bind> <it-pred> :body <thunk>)
	(list 'base-let (list 'it <it-bind>) (append (list 'while <it-pred>) <thunk> (ncons (list 'set! 'it <it-bind>)))))

; String functions
; if gen-string-index used container-ref 
; it could be used as a generic linear search function...
; or if it used functors; my only concern with that
; is with regards to compilation...
(define (gen-string-index proc s c idx)
	(cond
		((> idx (length s)) -1)
		((proc (nth s idx) c) idx)
		(else (gen-string-index proc s c (+ idx 1)))))
(define (string-index s c) (gen-string-index char=? s c 0))
(define (string-index-ci s c) (gen-string-index char-ci=? s c 0))

; first/rest compositions similar to cNr, up to four levels
(def ffirst (compose first first))
(def frest (compose first rest))
(def rfirst (compose rest first))
(def rrest (compose rest rest))
(def fffirst (compose first first first))
(def ffrest (compose first first rest))
(def frfirst (compose first rest first))
(def frrest (compose first rest rest))
(def rrrest (compose rest rest rest))
(def ffffirst (compose first first first first))
(def fffrest (compose first first first rest))
(def ffrfirst (compose first first rest first))
(def ffrrest (compose first first rest rest))
(def frffirst (compose first rest first first))
(def frfrest (compose first rest first rest))
(def frrfirst (compose first rest rest first))
(def frrrest (compose first rest rest rest))
(def rfffirst (compose rest first first first))
(def rffrest (compose rest first first rest))
(def rfrfirst (compose rest first rest first))
(def rfrrest (compose rest first rest rest))
(def rrffirst (compose rest rest first first))
(def rrfrest (compose rest rest first rest))
(def rrrfirst (compose rest rest rest first))
(def rrrrest (compose rest rest rest rest))

; since F now has a generic collection set, implement
; {string vector dict}-set! & {string vector dict}-ref
; for Scheme compatibility...
(define (string-ref s idx)
	(if (and (string? s) (integer? idx)) (nth s idx)))
(define (vector-ref v idx)
	(if (and (vector? v) (integer? idx)) (nth v idx)))
(define (dict-ref d idx)
	(if (and (dict? d) (or (string? idx) (symbol? idx) (key? idx))) (nth d idx)))
(define (list-ref l idx)
	(if (and (pair? l) (integer? idx)) (nth l idx)))
	
(define (string-set! s idx new)
	(if (and (string? s) (integer? idx)) (cset! s idx new)))
(define (vector-set! v idx new)
	(if (and (vector? v) (integer? idx)) (cset! v idx new)))
(define (dict-set! d idx new)
	(if (and (dict? d) (or (string? idx) (symbol? idx) (key? idx))) (cset! d idx new)))
(define (list-set! l idx new)
	(if (and (pair? l) (integer? idx)) (cset! l idx new)))
	
(define (char->int x) (coerce x 'int))
(define (int->char x) (coerce x 'char))

;; Some compat with other Scheme systems
(def exit quit)
