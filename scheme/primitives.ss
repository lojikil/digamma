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
;:quote #t
:length [1 #f "flength"] 
;:def #t
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
;:list [0 #f "list"]
;:vector #t
:make-vector [0 #f "fmkvector"]
:make-string [0 #f "fmakestring"]
;:string #t
:append [0 #f "fappend"]
:first [0 #f "ffirst"]
:rest [0 #f "frest"]
:ccons [0 #f "fccons"] 
:nth [0 #f "fnth"]
:keys #t 
:partial-key? #t
:cset! [0 #f "fcset"]
:empty? #t
:define-macro #t
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
:define-syntax #t
:string-append #t
:apply #t
:assq #t
:defrec #t
:set-rec! #t
:dict #t
:make-dict #t
:dict-has? #t
:clone-environment #t
:bind-environment #t
:set-environment! #t
:default-environment #t
:null-environment #t
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
:reset #t
:shift #t
:call/cc #t
:current-tick #t
})
(def primitive-form? (fn (x)
	(dict-has? *primitives* x)))
(def count-cdr (fn (obj n)
		(if (= n 0)
		 obj
		 (string-append "cdr(" (count-cdr obj (- n 1)) ")" ))))
(def gen-begin (fn (l)
		(if (eq? (cdr l) '())
		 (string-append "__return(" (gen-code (car l)) ");\n")
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
