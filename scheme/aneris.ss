; a simple digamma interpreter, to test out E'
; dead simple, uses the Vesta run time
(def *tlenv* (list
              (dict
               :car car
               :cdr cdr
               :cons cons
               :+ +
               :- -
               :* *
               :/ /)))
(def aneris@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (list #t (nth (car env) sym))
	  (aneris@lookup sym (cdr env))))))
(def aneris@apply (fn (proc args env)
    #f))
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
		 (with r (aneris@eval (car s))
		  (aneris@apply r (aneris@evlis (cdr s)) env))
		 (with r (aneris@lookup (car s))
		  (aneris@apply r (aneris@evlis (cdr s) env) env)))
	 else s)))
(def aneris@repl (fn ()
    (display "; ")
    (display (aneris@eval (read) *tlenv*))
    (newline)
    (aneris@repl)))
(def aneris@main (fn ()
    (display "aneris r0\n")
    (aneris@repl)))    
