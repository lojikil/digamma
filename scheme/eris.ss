; A Digamma->C compiler that supports the full Digamma spec (including POSIX & W7).
; it's probably a crappy method of compilation, but it eases transition, since it
; reuses Vesta's runtime. The Best method would be to have a nice Type inference system
; that uses something similar to a tagged pointer for SExprs, rather than the heavy struct/union
; I use now. On top of that, I could unbox many things; this is definitely the tact I will take
; with future releases, but for now I'm just looking to speed up Digamma development & run times 
; :D
; zlib/png licensed (c) 2010 Stefan Edwards

(load "primitives.ss")

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
(def gen-literal (fn (x)
	(cond
		(number? x) (gen-number x)
		(string? x) (gen-string x)
		(vector? x) (gen-vector x)
		(pair? x) (gen-pair x) ; really, need to tell what type of code to generate here...
		(dict? x) (gen-dict x)
		(symbol? x) (gen-symbol x)
		else (error "unsupported file type for code generation"))))
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
							'LIFT-LAMBDAS)))		
			(eq? (car x) 'load) #t
			(eq? (car x) 'import) #t
			(eq? (car x) 'use) #t
			(eq? (car x) 'from) #t
			(eq? (car x) 'quote) (gen-literal (car (cdr x)))
			(pair? (car x)) #t
			(primitive-form? (car x)) (gen-primitive x) 
			else 'EVAL-FORM)
		(gen-literal x))))

; The basic system is this:
; - read form from file
; - call syntax-expand
; - call macro-expand
; - call gen-code on what's left
; - loop to the first item until #e
