;; A simple stack-based VM for Digamma
;; Meant to support both interpretation in Vesta as
;; well as compilation with E'
;; copyright 2011 Stefan Edwards; please see the LICENSE
;; file for details

;; TODO:
;; - good compilation mechanism for hydra@eval
;; - method for vm@eval to manage things like (cons (car (cons 1 2)) (cdr (1 2)))
;;   which it cannot currently do because we need to rotate the stack (wait, do we?)
;;   (cons (car (cons 1 '())) (cdr 4 '())):
;;   (4)   ;; nil
;;   (3 4) ;; load 4
;;   (2)   ;; cons
;;   (1)   ;; cdr
;;   (4)   ;; nil
;;   (3 1) ;; load 1
;;   (2)   ;; cons
;;   (0)   ;; car
;;   (2)   ;; cons
;;   works, so this isn't blocked. \o/


(define (vm@instruction c)
    (car c))

(define (vm@operand c)
    (cadr c))

(define (vm@eval code env (stack '()))
     " process the actual instructions of a code object; the basic idea is that
       the user enters:
       h; (car (cdr (cons 1 (cons 2 '()))))
       which is compiled by hydra@eval into:
       (4)   ;; nil
       (3 2) ;; load 2
       (2)   ;; cons
       (3 1) ;; load 1
       (2)   ;; cons
       (1)   ;; cdr
       (0)   ;; car
       
       which vm@eval can then interpret in a tail-call fashion.
       There might be better ways to store the actual VM codes themselves
       other than pairs of pairs (even vector of pairs would be more efficient really)
       and it might be worth it to add two collexion-neutral primitives, cappend & 
       cappend!, to the collexion API. Also, adding named-enumerations to the language,
       even if at the syntactic level, would be helpful. If (enumerate OPCAR OPCDR ...)
       even compiled to
       #define OPCAR 0
       #define OPCDR 1
       // ...
       
       It would be more useful, and the names could be used throughout (and checked!)."
     (if (null? code)
         (car stack)
         (let* ((c (car code))
                (instr (vm@instruction c))
                (oper  (vm@operand c))) ;; this is simple to transition to register vm
              (cond ;; case would make a lot of sense here...
                  (eq? instr 0) ;; car
                        (vm@eval (cdr code)
                                 env
                                 (cons (car (car stack)) (cdr stack)))
                  (eq? instr 1) ;; cdr
                        (vm@eval (cdr code)
                                 env
                                 (cons (cdr (car stack)) (cdr stack)))
                  (eq? instr 2) ;; cons
                        (vm@eval (cdr code)
                                 env
                                 (cons (cons (car stack)
                                                (cadr stack))
                                       (cddr stack)))
                  (eq? instr 3) ;; load
                        (vm@eval (cdr code)
                                 env
                                 (cons oper stack))
                  (eq? instr 4) ;; nil
                        (vm@eval (cdr code)
                                 env
                                 (cons '() stack))
                  (eq? instr 5) ;; -
                        (vm@eval (cdr code)
                                 env
                                 (cons (- (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 6) ;; +
                        (vm@eval (cdr code)
                                 env
                                 (cons (+ (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 7) ;; * 
                        (vm@eval (cdr code)
                                 env
                                 (cons (* (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 8) ;; / 
                        (vm@eval (cdr code)
                                 env
                                 (cons (/ (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 9) ;;  < 
                        (vm@eval (cdr code)
                                 env
                                 (cons (< (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 10) ;; >
                        (vm@eval (cdr code)
                                 env
                                 (cons (> (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 11) ;; <= 
                        (vm@eval (cdr code)
                                 env
                                 (cons (<= (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 12) ;; >= 
                        (vm@eval (cdr code)
                                 env
                                 (cons (>= (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 26) ;; = 
                        (vm@eval (cdr code)
                                 env
                                 (cons (= (car stack) (cadr stack)) (cddr stack)))))))

(define *tlenv* '({
    :car 0
    :cdr 1
    :cons 2
    :- 5
    :+ 6
    :* 7
    :/ 8
    :< 9
    :> 10
    :<= 11
    :>= 12
    :if primitive-syntax-if 
    :fn primitive-syntax-fn
    :lambda primitive-syntax-fn
    :quote primitive-syntax-quote
    :quasi-quote primitive-syntax-qquote
    :unquote primitve-syntax-unquote
    :unquote-splice primitive-syntax-unqsplice
    :eval 13
    :load 14
    :apply 15
    :display 16
    :write 17
    :read 18
    :read-string 19
    :read-char 20
    :write-string 21
    :write-char 22
    :write-buffer 23
    :numerator 24
    :denomenator 25
}))

(define (hydra@lookup item env)
    " look up item in the current environment, returning #f for not found"
    (cond
        (null? env) #f
        (dict-has? (car env) item) (nth (car env) item)
        else (hydra@lookup item (cdr env))))

(define (hydra@lambda? x)
    #f)

(define (reverse-append x)
    "append but in reverse; not currently working, but close"
    (cond
        (null? x) x
        (null? (cdr x)) (car x)
        else (append (reverse-append (cddr x)) (cadr x) (car x))))

(define (show x) (display "show: ") (display x) (newline) x)

(define (hydra@eval line env (thusfar '()))
    (display (format "~a; ~a~%" line thusfar))
    (if (null? line)
        thusfar
        (cond
            (vector? line) (list 'load line)
            (dict? line) (list 'load line) 
            (pair? line) 
                (let* ((fst (car line)) ;; decompose line into first & rest
                       (v (hydra@lookup fst env)) ;; find fst in env
                       (rst (cdr line))) 
                   (display "in hydra@eval let*\n")
                   (if (eq? fst #f) ;; failed to find fst
                       (error (format "Symbol not found: ~a~%" fst)) 
                       (cond 
                            (symbol? v) ;; primitive syntax
                                (cond
                                    (eq? v 'primitive-syntax-quote)
                                        (if (null? (cadr rst))
                                            '(4)
                                            (list 'load (cadr rst)))
                                    else #t)
                            (integer? v) ;; primitive procedure
                                ;; need to generate the list of HLAP code, reverse it
                                ;; and flatten it. basically, if we have:
                                ;; (cons (+ 1 2) (cons (+ 3 4) '()))
                                ;; we need to:
                                ;; (quote ())
                                ;; (+ 3 4)
                                ;; (cons)
                                ;; (+ 1 2)
                                ;; (cons)
                                ;; this isn't the *most* efficient, but it is pretty easy
                                (show (cons
                                    (reverse-append
                                        (map (fn (x) (show (hydra@eval x env)))
                                             rst))
                                    (list v)))
                            (hydra@lambda? v) ;; hydra closure
                                #t
                            else (error "error: the only applicable types are primitive procedures, closures & syntax"))))

            else (cons (cons 'load (cons line '())) thusfar))))

(define (hydra@repl)
    (display "h; ")
    (with inp (read)
     (if (and (eq? (type inp) "Pair") (eq? (car inp) 'unquote))
        (cond
         (eq? (cadr inp) 'exit) (quit)
         (eq? (cadr inp) 'quit) (quit)
         (eq? (cadr inp) 'dribble) #t
         (eq? (cadr inp) 'save) #t
         else #f)
        (with r (hydra@eval inp *tlenv*)
            (if (not (eq? r #v))
                (begin
                    (write r)
                    (newline))
                #v)
            (hydra@repl)))))

(define (hydra@main)
    (display "\n\t()\n\t  ()\n\t()  ()\nDigamma/Hydra: 2009.3/r0\n")
    (hydra@repl))    
