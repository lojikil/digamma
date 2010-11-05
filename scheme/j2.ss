(def member? (fn (x lst)
	(if (eq? (car lst) x) 
		#t
		(if (eq? lst '())
		#f
		(member? x (cdr lst))))))
