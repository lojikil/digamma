; a simple digamma subset interpreter, to test out E'
; dead simple, uses the Vesta run time
; Ceres could be forked from here...

(def *tlenv* (list
              (dict
               :car :pcar
               :cdr :pcdr
               :cons :pcons
               :+ :p+
               :- :p-
               :* :p*
               :/ :p/)))
(def aneris@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (nth (car env) sym)
	  (aneris@lookup sym (cdr env))))))
(def aneris@apply (fn (proc args env)
    (cond
     (eq? proc #f) (begin (display "Aneris error: unknown procedure\n") #v)
     (eq? (type proc) "Vector") #t ; lambda
     (eq? proc :pcar) (car (car args))
     (eq? proc :pcdr) (cdr (car args))
     (eq? proc :pcons) (cons (car args) (car (cdr args)))
     (eq? proc :p+) (foldl + 0 args)
     (eq? proc :p-) (foldl - (car args) (cdr args))
     (eq? proc :p*) (foldl * 1 args)
     (eq? proc :p/) (foldl / 1 args)
     else #f)))
(def aneris@evlis (fn (args builtlist env)
    (if (eq? args '())
     builtlist
     (if (eq? (type (car args)) "Pair")
        (aneris@evlis (cdr args) (append builtlist (list (aneris@eval (car args) env))) env)
        (aneris@evlis (cdr args) (append builtlist (list (car args))) env)))))
(def aneris@eval (fn (s e)
	(cond
	 (eq? (type s) "Symbol")
	 	(with r (aneris@lookup s e)
		 (if (eq? r #f)
		  (begin (display (format "error, No such symbol: ~a" s)) #v)
		  (car (cdr r))))
	 (eq? (type s) "Pair") 
	 	(if (eq? (type (car s)) "Pair")
		 (with r (aneris@eval (car s) e)
		  (aneris@apply r (aneris@evlis (cdr s) '() e) e))
                 (cond
                  (eq? (car s) 'quote) (car (cdr s))
                  (eq? (car s) 'quasiquote) #t
                  (eq? (car s) 'if) #t
                  (eq? (car s) 'let) #t
                  (eq? (car s) 'fn) #t
                  (eq? (car s) 'def) #t
                  (eq? (car s) 'set! #t
                  (eq? (car s) 'cond) #t 
		  else (with r (aneris@lookup (car s) e)
		   (aneris@apply r (aneris@evlis (cdr s) '() e) e))))
	 else s)))
(def aneris@repl (fn ()
    (display "; ")
    (display (aneris@eval (read) *tlenv*))
    (newline)
    (aneris@repl)))
(def aneris@main (fn ()
    (display "aneris r0\n")
    (aneris@repl)))    
