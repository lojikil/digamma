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
(def nyx@compare (fn (c s r) ; really need to add cond to rewrite-tail-call
    (cond
     (eq? r '()) #t
     (eq? c :p=) (if (= s (car r)) (nyx@compare c (car r) (cdr r)) #f)
     (eq? c :p<) (if (< s (car r)) (nyx@compare c (car r) (cdr r)) #f)
     (eq? c :p>) (if (> s (car r)) (nyx@compare c (car r) (cdr r)) #f)
     (eq? c :p<=) (if (<= s (car r)) (nyx@compare c (car r) (cdr r)) #f)
     (eq? c :p>=) (if (>= s (car r)) (nyx@compare c (car r) (cdr r)) #f)
     else #f)))
(def nyx@lookup (fn (sym env)
	(if (eq? env '())
	 #f
	 (if (dict-has? (car env) sym)
	  (nth (car env) sym)
	  (nyx@lookup sym (cdr env))))))
(def nyx@logop? (fn (p)
    (cond
     (eq? p :p<) #t
     (eq? p :p>) #t
     (eq? p :p=) #t
     (eq? p :p<=) #t
     (eq? p :p>=) #t
     else #f)))
(def nyx@qquote (fn (l e)
    (if (eq? (type l) "Pair")
     (if (eq? (type (car l)) "Pair")
      (if (eq? (car (car l)) 'unquote)
       (cons (nyx@eval (car (cdr (car l))) e) (nyx@qquote (cdr l) e))
       (if (eq? (car (car l)) 'unquote-splice)
        (append (nyx@eval (car (cdr (car l))) e) (nyx@qquote (cdr l) e))          
        (cons (nyx@qquote (car l) e) (nyx@qquote (cdr l) e))))
      (cons (car l) (nyx@qquote (cdr l) e)))
     l)))
(def nyx@apply (fn (proc args env)
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
     (eq? proc :pqquote) (nyx@qquote (car args) env) 
     (eq? proc :pif) (if (not (eq? (nyx@eval (car args) env) #f)) (nyx@eval (car (cdr args)) env) (nyx@eval (car (cdr (cdr args))) env))
     (eq? proc :pdisplay) (display (car args))
     (nyx@logop? proc) (nyx@compare proc (car args) (cdr args))
     else #f)))
(def nyx@evlis (fn (args builtlist env)
    (if (eq? args '())
     builtlist
     (if (eq? (type (car args)) "Pair")
        (nyx@evlis (cdr args) (append builtlist (list (nyx@eval (car args) env))) env)
        (nyx@evlis (cdr args) (append builtlist (list (car args))) env)))))
(def nyx@eval (fn (s e)
	(cond
	 (eq? (type s) "Symbol")
	 	(with r (nyx@lookup s e)
		 (if (eq? r #f)
		  (begin (display (format "error, No such symbol: ~a~%" s)) #v)
		  (if (eq? (type r) "Vector")
		  	(format "#<primitive ~a>" (nth r 1))
		      (car (cdr r)))))	
	 (eq? (type s) "Pair") 
	 	(if (eq? (type (car s)) "Pair")
		 (with r (nyx@eval (car s) e)
		  (nyx@apply r (nyx@evlis (cdr s) '() e) e))
		  (with r (nyx@lookup (car s) e)
                   (if (not r)
                    (begin (display (format "error, no such applicable: ~a\n" (car s))) #v)
                    (if (= (nth r 0) 1)
                     (nyx@apply (nth r 1) (cdr s) e)
		     (nyx@apply (nth r 1) (nyx@evlis (cdr s) '() e) e)))))
	 else s)))
(def nyx@repl (fn ()
    (display "a; ")
    (with r (nyx@eval (read) *tlenv*)
     (if (not (eq? r #v))
      (begin
       (write r)
       (newline))
      #v))
    (nyx@repl)))
(def nyx@main (fn ()
    (display "nyx r0\n")
    (nyx@repl)))    
