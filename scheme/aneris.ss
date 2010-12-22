; a simple digamma interpreter, to test out E'
; dead simple, uses the Vesta run time
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
	  (list #t (nth (car env) sym))
	  (aneris@lookup sym (cdr env))))))
(def aneris@apply (fn (proc args env)
    (display "proc = ")
    (display proc)
    (newline)
    (cond
     (eq? proc #f) (begin (display "Aneris error: unknown procedure\n") #v)
     (eq? (cadr proc) :pcar) (car (car args))
     (eq? (cadr proc) :pcdr) (cdr (car args))
     (eq? (cadr proc) :pcons) (cons (car args) (car (cdr args)))
     (eq? (cadr proc) :p+) (foldl + 0 args)
     (eq? (cadr proc) :p-) (foldl - (car args) (cdr args))
     (eq? (cadr proc) :p*) (foldl * 1 args)
     (eq? (cadr proc) :p/) (foldl / 1 args)
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
		 (with r (aneris@lookup (car s) e)
		  (aneris@apply r (aneris@evlis (cdr s) '() e) e)))
	 else s)))
(def aneris@repl (fn ()
    (display "; ")
    (display (aneris@eval (read) *tlenv*))
    (newline)
    (aneris@repl)))
(def aneris@main (fn ()
    (display "aneris r0\n")
    (aneris@repl)))    
