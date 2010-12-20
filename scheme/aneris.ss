; a simple digamma interpreter, to test out E'
; dead simple, uses the Vesta run time
(def *tlenv* (list
              (dict
               :car car
               :cdr cdr
               :cons cons
               :+ +
               :- -))) 
(def aneris@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (list #t (nth (car env) sym))
	  (aneris@lookup sym (cdr env))))))
(def aneris@apply (fn (proc args env)
    #f))
(def aneris@evlis (fn (args env)
    #f))
(def aneris@eval (fn (s e)
	(cond
	 (eq? (type s) "Symbol")
	 	(with r (aneris@lookup s e)
		 (if (eq? r #f)
		  (error (format "No such symbol: ~a" s))
		  (car (cdr r))))
	 (eq? (type s) "Pair") #f
	 else s)))
(def aneris@repl (fn ()
    (display "; ")
    (display (aneris@eval (read) *tlenv*))
    (newline)
    (aneris@repl)))
(def aneris@main (fn ()
    (display "aneris r0\n")
    (aneris@repl)))    
