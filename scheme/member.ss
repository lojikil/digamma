(load "./prelude.ss")
(def member? (fn (x lst)
	(display (format "x => ~a ; lst => ~a~%" x lst))
	(cond
		(eq? (car lst) x) #t
		(eq? lst '()) #f
		else (member? x (cdr lst)))))
(display (member? 'beer '(apple cheese pizza pie beer)))
(newline)
