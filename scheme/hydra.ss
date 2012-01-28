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
;; - lambda lifting: it would be nice if lambdas were lifted to avoid allocation as
;;   much as possible
;; - define-instruction syntax that can be used to populate vm@eval as well as 
;;   clean up redundancies in code (like manual calls to vm@eval in each instruction)

(define (vm@instruction c)
    (car c))

(define (vm@operand c)
    (cadr c))

(define (vm@eval code env (ip 0) (stack '()))
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
     ;; if this is moved to an IP-based (instruction pointer)
     ;; system, it might be easier to do this using a named-let rather than iterating 
     ;; at the top level. E' should be able to lift named-lets pretty easily into whiles
     ;; and that would fit pretty well here. Also, moving to an inner-loop with named-let
     ;; means I can alliviate some of the duplication here; code & env are rarely modified
     ;; so it would also make sense to not have to pass them on every call. A lot of
     ;; duplication...
     ;; also, I would really like to have these all defined using a Scheme48-style
     ;; define-operator, since that would be much cleaner than what is seen below.
     ;; Syntax could expand the full list of operators in place here, and it would make
     ;; expanding the set of operators *much* easier than it currently is.
     ;(display "stack: ")
     ;(display stack)
     ;(newline)
     ;(display "code: ") 
     ;(display code)
     ;(newline)
     ;(display "ip: ")
     ;(display ip)
     ;(newline)
     (if (>= ip (length code))
         (car stack)
         (let* ((c (nth code ip))
                (instr (vm@instruction c)))
              (cond ;; case would make a lot of sense here...
                  (eq? instr 0) ;; car
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (car (car stack)) (cdr stack)))
                  (eq? instr 1) ;; cdr
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (cdr (car stack)) (cdr stack)))
                  (eq? instr 2) ;; cons
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (cons (car stack)
                                                (cadr stack))
                                       (cddr stack)))
                  (eq? instr 3) ;; load
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (vm@operand c) stack))
                  (eq? instr 4) ;; nil
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons '() stack))
                  (eq? instr 5) ;; -
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (- (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 6) ;; +
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (+ (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 7) ;; * 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (* (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 8) ;; / 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (/ (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 9) ;;  < 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (< (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 10) ;; >
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (> (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 11) ;; <= 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (<= (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 12) ;; >= 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (>= (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 26) ;; = 
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (= (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 27) ;; eq?
                        (vm@eval code
                                 env
                                 (+ ip 1)
                                 (cons (eq? (car stack) (cadr stack)) (cddr stack)))
                  (eq? instr 28) ;; jump
                        (vm@eval code
                                 env
                                 (+ ip (vm@operand c))
                                 stack)
                  (eq? instr 29) ;; cmp
                        (if (car stack) ;; if the top of the stack is true
                            (vm@eval code env (+ ip 1) (cdr stack)) ;; jump to the <then> portion
                            (vm@eval code env (+ ip (vm@operand c)) (cdr stack)))))))

; syntax to make the above nicer:
; (define-instruction := "numeq" '() '() (+ ip 1) (cons (= (car stack) (cadr stack)) (cddr stack)))
; (define-instruction '() "jump" '() '() '() (if (car stack) ...))
; this should also fill in the top-level environment for those that have non-null name
; (define-instruction name short-description code-manip stack-manip IP-manip resulting code) 

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
    := 26
    :eq? 27
    ;; 28 is jump
    ;; 29 is compare
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
    (if (null? line)
        thusfar
        (cond
            (vector? line) (list 3 line)
            (dict? line) (list 3 line) 
            (pair? line) 
                (let* ((fst (car line)) ;; decompose line into first & rest
                       (v (hydra@lookup fst env)) ;; find fst in env
                       (rst (cdr line))) 
                   (if (eq? fst #f) ;; failed to find fst
                       (error (format "Symbol not found: ~a~%" fst)) 
                       (cond 
                            (symbol? v) ;; primitive syntax
                                (cond
                                    (eq? v 'primitive-syntax-quote)
                                        (if (null? (car rst))
                                            '((4))
                                            (list (list 3 (car rst))))
                                    (eq? v 'primitive-syntax-if)
                                        ;; need to generate code for <cond>
                                        ;; add CMP instruction '(30)
                                        ;; generate code for <then>
                                        ;; generate code for <else>
                                        ;; add count to CMP instruction to jump to <else>
                                        ;; add count to <then> to skip <else>
                                        (let* ((<cond> (hydra@eval (car rst) env))
                                               (<then> (hydra@eval (cadr rst) env))
                                               (<else> (hydra@eval (caddr rst) env))
                                               (then-len (+ (length <then>) 2)) ;; +2 in order to avoid the jump over else
                                               (else-len (+ (length <else>) 1)))
                                            (append <cond>
                                                (list (list 29 then-len)) ;; compare & jump
                                                <then>
                                                (list (list 28 else-len)) ;; jump else
                                                <else>)) 
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
                                (append
                                    (reverse-append
                                        (map (fn (x) (hydra@eval x env))
                                             rst))
                                    (list (list v)))
                            (hydra@lambda? v) ;; hydra closure
                                #t
                            else (error "error: the only applicable types are primitive procedures, closures & syntax"))))

            else (cons (cons 3 (cons line '())) thusfar))))

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
        (with r (vm@eval (list->vector (hydra@eval inp *tlenv*)) *tlenv*)
            (if (not (eq? r #v))
                (begin
                    (write r)
                    (newline))
                #v)
            (hydra@repl)))))

(define (hydra@main)
    (display "\n\t()\n\t  ()\n\t()  ()\nDigamma/Hydra: 2009.3/r0\n")
    (hydra@repl))    
