; a simple digamma interpreter, to test out E'
; dead simple, uses the Vesta run time

(def aneris@eval (fn (s e)
	(cond
	 (eq? (type s) 'symbol) (aneris@lookup s e)
