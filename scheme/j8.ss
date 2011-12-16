(def (member0? x l)
 (cond
  (eq? l '()) #f
  (eq? (car l) x) #t
  else (member0? x (cdr l))))
