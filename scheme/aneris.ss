; a simple digamma interpreter, to test out E'
; dead simple, uses the Vesta run time
(def aneris@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (list #t (nth (car env) sym))
	  (aneris@lookup sym (cdr env))))))
(def aneris@eval (fn (s e)
	(cond
	 (eq? (type s) 'symbol)
	 	(with r (aneris@lookup s e)
		 (if (eq? r #f)
		  (error (format "No such symbol: ~a" s))
		  (car (cdr r))))
	 (eq? (type s) 'pair) #f
	 else s)))
(def aneris@main (fn ()
	
