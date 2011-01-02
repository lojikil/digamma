; a simple digamma subset interpreter, to test out E'
; heading towards full Digamma compliance though
; will add W7 & delimited continuations before the new year (2011)

(def *tlenv* (list
              (dict
               :car [0 :pcar]
               :cdr [0 :pcdr]
               :cons [0 :pcons]
               :fn [1 :pfn]
               :if [1 :pif]
               :quote [1 :pquote]
               :quasiquote [1 :pqquote]
               :def [1 :pdef]
               :set! [1 :pset!]
               :list [0 :plist]
               :length [0 :plength]
               :display [0 :pdisplay]
               :< [0 :p<]
               :> [0 :p>]
               :<= [0 :p<=]
               :>= [0 :p>=]
               := [0 :p=]
               :+ [0 :p+]
               :- [0 :p-]
               :* [0 :p*]
               :/ [0 :p/])))
(def aneris@compare (fn (c s r) ; really need to add cond to rewrite-tail-call
    (cond
     (eq? r '()) #t
     (eq? c :p=) (if (= s (car r)) (aneris@compare c (car r) (cdr r)) #f)
     (eq? c :p<) (if (< s (car r)) (aneris@compare c (car r) (cdr r)) #f)
     (eq? c :p>) (if (> s (car r)) (aneris@compare c (car r) (cdr r)) #f)
     (eq? c :p<=) (if (<= s (car r)) (aneris@compare c (car r) (cdr r)) #f)
     (eq? c :p>=) (if (>= s (car r)) (aneris@compare c (car r) (cdr r)) #f)
     else #f)))
(def aneris@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (nth (car env) sym)
	  (aneris@lookup sym (cdr env))))))
(def aneris@logop? (fn (p)
    (cond
     (eq? p :p<) #t
     (eq? p :p>) #t
     (eq? p :p=) #t
     (eq? p :p<=) #t
     (eq? p :p>=) #t
     else #f)))
(def aneris@qquote (fn (l e)
    (if (eq? (type l) "Pair")
     (if (eq? (type (car l)) "Pair")
      (if (eq? (car (car l)) 'unquote)
       (cons (aneris@eval (car (cdr (car l))) e) (aneris@qquote (cdr l) e))
       (if (eq? (car (car l)) 'unquote-splice)
        (append (aneris@eval (car (cdr (car l))) e) (aneris@qquote (cdr l) e))          
        (cons (aneris@qquote (car l) e) (aneris@qquote (cdr l) e))))
      (cons (car l) (aneris@qquote (cdr l) e)))
     l)))
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
     (eq? proc :pquote) (car args)
     (eq? proc :pqquote) (aneris@qquote (car args) env) 
     (eq? proc :pif) (if (not (eq? (aneris@eval (car args) env) #f)) (aneris@eval (car (cdr args)) env) (aneris@eval (car (cdr (cdr args))) env))
     (eq? proc :pdisplay) (display (car args))
     (aneris@logop? proc) (aneris@compare proc (car args) (cdr args))
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
		  (begin (display (format "error, No such symbol: ~a~%" s)) #v)
		  (if (eq? (type r) "Vector")
		  	(format "#<primitive ~a>" (nth r 1))
		      (car (cdr r)))))	
	 (eq? (type s) "Pair") 
	 	(if (eq? (type (car s)) "Pair")
		 (with r (aneris@eval (car s) e)
		  (aneris@apply r (aneris@evlis (cdr s) '() e) e))
		  (with r (aneris@lookup (car s) e)
                   (if (not r)
                    (begin (display (format "error, no such applicable: ~a\n" (car s))) #v)
                    (if (= (nth r 0) 1)
                     (aneris@apply (nth r 1) (cdr s) e)
		     (aneris@apply (nth r 1) (aneris@evlis (cdr s) '() e) e)))))
	 else s)))
(def aneris@repl (fn ()
    (display "a; ")
    (with r (aneris@eval (read) *tlenv*)
     (if (not (eq? r #v))
      (begin
       (write r)
       (newline))
      #v))
    (aneris@repl)))
(def aneris@main (fn ()
    (display "aneris r0\n")
    (aneris@repl)))    
