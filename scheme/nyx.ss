; a simple digamma subset interpreter, to test out E'
; heading towards full Digamma compliance though
; will add W7 & delimited continuations before the new year (2011)

(def *tlenv* (list
              (dict
               :car [0 :pcar]
               :cdr [0 :pcdr]
               :cons [0 :pcons]
               :fn [1 :pfn]
               :lambda [1 :pfn]
               :if [1 :pif]
               :quote [1 :pquote]
               :quasiquote [1 :pqquote]
               :def [1 :pdef]
               :define [1 :pdef]
               :set! [1 :pset!]
               :list [0 :plist]
               :length [0 :plength]
               :nth [0 :pnth]
               :display [0 :pdisplay]
               :define-syntax [1 :pdefsyn]
               :define-macro [1 :pdefmac]
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
       (cons (nyx@eval (car (cdr (car l))) :preapply '() e) (nyx@qquote (cdr l) e))
       (if (eq? (car (car l)) 'unquote-splice)
        (append (nyx@eval (car (cdr (car l))) :preapply '() e) (nyx@qquote (cdr l) e))          
        (cons (nyx@qquote (car l) e) (nyx@qquote (cdr l) e))))
      (cons (car l) (nyx@qquote (cdr l) e)))
     l)))
(def nyx@eval (fn (s state stack e)
    (cond 
     (eq? (type s) "Symbol") 
        (with res (nyx@lookup s e)
         (if (eq? res #f)
          (begin 
           (display (format "Undefined Symbol: ~a\n" s))
           #v)
          res))
     (not (eq? (type s) "Pair")) s
     (eq? state :preapply) 
     	(with fst (nyx@lookup (car s) e)
	        (if (eq? fst #f)
	            (begin 
	                (display (format "Undefined symbol, ~a in ~A\n" (car s) s))
	                #v)
	            (cond
	                (eq? (type fst) "Vector") 
                        (if (= (nth fst 0) 0)
                         (nyx@eval (cons fst (cdr s)) :postapply stack e)
                         (nyx@eval (cdr s) (nth fst 1) stack e))
	                (eq? (type fst) "Dict") #t
	                 else (display "Some sort of error...\n"))))
     (eq? state :postapply) #t
     (eq? state :preturn) #t
     (eq? state :pcar) 
        (if (eq? (type (car s)) "Pair")
         (car (car s))
         (begin
          (display "Type error: car operates on PAIRs only!\n")
          #v))
     (eq? state :pcdr) 
        (if (eq? (type (car s)) "Pair")
         (cdr (car s))
         (begin
          (display "Type error: cdr operates on PAIRs only!\n")
          #v))
     (eq? state :pcons) 
	  (cons (caar s) (cadar s))
     (eq? state :pfn) #t
     (eq? state :pif) #t
     (eq? state :pquote) (car s)
     (eq? state :pqquote) (nyx@qquote (car s) e)
     (eq? state :pdef) #t
     (eq? state :pset!) #t
     (eq? state :pdef) #t
     (eq? state :plist) #t
     (eq? state :plength) #t
     (eq? state :pnth) #t
     (eq? state :display) #t
     (eq? state :p<) #t
     (eq? state :p>) #t
     (eq? state :p<=) #t
     (eq? state :p>=) #t
     (eq? state :p=) #t
     (eq? state :p-) #t
     (eq? state :p+) #t
     (eq? state :p/) #t
     (eq? state :p*) #t
     else (error "Invalid state"))))
(def nyx@repl (fn ()
    (display "n; ")
    (with inp (read)
     (if (and (eq? (type inp) "Pair") (eq? (car inp) 'unquote))
        (cond
         (eq? (cadr inp) 'exit) (quit)
         (eq? (cadr inp) 'quit) (quit)
         (eq? (cadr inp) 'dribble) #t
         (eq? (cadr inp) 'save) #t
         else #f)
        (with r (nyx@eval inp :preapply '() *tlenv*)
            (if (not (eq? r #v))
                (begin
                    (write r)
                    (newline))
                #v)
            (nyx@repl))))))
(def nyx@main (fn ()
    (display "\n\t()\n\t  ()\n\t()  ()\nDigamma/Nyx: 2009.3/r0\n")
    (nyx@repl)))    
