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
        (append (nyx@eval (car (cdr (car l))) :preapply '() e) (nyx@qquote (cdr l) e))          
        (cons (nyx@qquote (car l) e) (nyx@qquote (cdr l) e))))
      (cons (car l) (nyx@qquote (cdr l) e)))
     l)))
(def nyx@eval (fn (s state stack e)
#f))
(def nyx@repl (fn ()
    (display "a; ")
    (with r (nyx@eval (read) :preapply '() *tlenv*)
     (if (not (eq? r #v))
      (begin
       (write r)
       (newline))
      #v))
    (nyx@repl)))
(def nyx@main (fn ()
    (display "nyx r0\n")
    (nyx@repl)))    
