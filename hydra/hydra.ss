;; A simple stack-based VM for Digamma
;; Meant to support both interpretation in Vesta as
;; well as compilation with E'
;; copyright 2011 Stefan Edwards; please see the LICENSE
;; file for details

;; TODO:
;; - DONE: good compilation mechanism for hydra@eval
;; - DONE: method for hydra@vm to manage things like (cons (car (cons 1 2)) (cdr (1 2)))
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
;; - define-instruction syntax that can be used to populate hydra@vm as well as 
;;   clean up redundancies in code (like manual calls to hydra@vm in each instruction)
;; - define a clean method of boxed representations of types, one that can be used from
;;   Vesta or E'. Not sure if this is a decent use of time here, in a VM, since that 
;;   should be a function of the run time, not the interpreter (we wouldn't want Ceres
;;   & Hydra to have different representations, for instance). Still, it would be nice
;;   if errors & other types can be simply encoded '(error "error"); SRFI-9, esp. if it
;;   has E' support, might be a good option (need to unbox types in E' though, for the
;;   most efficient representation, as well as unions).
;; - Make all instructions support (type . value) pairs, so as to avoid a situation where
;;   the user enters the code (0 (list 1 2 3)) and recieves the value 1 back, because:
;;   (load 0)
;;   (call) ;; call integer 0, since integers -> primitives, this can add some weird behavior.
;; - DONE: Just noticed that the way the CALL operand is implemented, the stack will no longer
;;   hold the parameters. Need to walk over the params to a lambda & bind variables from the
;;   stack before moving to running the code... 
;; - add a debug variable, so that you could ,debug-engine at the REPL, and I wouldn't have to comment/uncomment
;;   debug lines every time I wanted to check out what's going on underneath the hood
;; - DONE fix this:
;;      h; 0
;;      #<primitive-procedure 0>
;;   it should be:
;;      h; 0
;;      0
;;      h; car
;;      #<primitive-procedure 0>
;;
;; - DONE: I wonder if this should re-write to %define, so that I don't have
;; to do anything fancy with eval... There are three cases:
;; (define f literal)
;; (define f (fn (x) (+ x x)))
;; (define f (car '(1 2 3 4 5)))
;; the first two *can* be handled OOB by hydra@eval, but the third
;; cannot really be handled properly. It should be re-written to 
;; (load (1 2 3 4 5))
;; (car)
;; (load f)
;; (%define)
;; - SRFIs to be added: 9, 22, 34, 35, 36, 57, 60, 89, 88 (already done, via Vesta's run time)

(define (list-copy l)
    " really, should be included from SRFI-1, but this simply makes a copy
      of the spine of a pair
      "
    (if (null? l)
        l
        (cons (car l) (list-copy (cdr l)))))

(define (cadddar x) (car (cdddar x)))

(define (hydra@instruction c)
    (car c))

(define (hydra@operand c)
    (cadr c))

(define (build-environment environment stack params)
    "Adds a new window to the environment, removes |params| items from the stack
     and binds those values in the new window. It returns a list of environment and
     the new stack."
    ;; rough match; doesn't take into account optional parameters.
    ;; would probably be better to have an inner function that iterates over
    ;; the parameters & returns those that match. It would then be easier to 
    ;; have optional parameters...
    (let ((ls (length stack)) (lp (length params)) (nu-env {}))
        (if (< ls lp)
            (error "non-optional parameters are not statisfied by stack items in build-environment")
            (if (= lp 0)
                (list (cons nu-env environment) (cdr stack))
                (begin 
                    (foreach-proc
                      (lambda (x)
                      (cset! nu-env (car x) (cadr x)))
                    (zip params (cslice stack 0 lp)))
                    (list (cons nu-env environment) (cslice stack lp ls)))))))

(define (hydra@vm code env (ip 0) (stack '()) (dump '()))
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
       
       which hydra@vm can then interpret in a tail-call fashion.
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
        (if (null? dump)
            (car stack)
            (hydra@vm (caar dump) (cadar dump) (+ (caddar dump) 1) (cons (car stack) (cadddar dump)) (cdr dump)))
         (let* ((c (nth code ip))
                (instr (hydra@instruction c)))

                ;(display "current instruction: ")
                ;(display (nth code ip))
                ;(newline)   
              (cond ;; case would make a lot of sense here...
                  (eq? instr 0) ;; car
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (car (car stack)) (cdr stack)) dump)
                  (eq? instr 1) ;; cdr
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (cdr (car stack)) (cdr stack)) dump)
                  (eq? instr 2) ;; cons
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (cons (car stack)
                                                (cadr stack))
                                       (cddr stack)) dump)
                  (eq? instr 3) ;; load
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (hydra@operand c) stack) dump)
                  (eq? instr 4) ;; nil
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons '() stack) dump)
                  (eq? instr 5) ;; -
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (- (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 6) ;; +
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (+ (car stack) (cadr stack)) (cddr stack)) dump)
                  (eq? instr 7) ;; * 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (* (car stack) (cadr stack)) (cddr stack)) dump)
                  (eq? instr 8) ;; / 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (/ (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 9) ;;  < 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (< (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 10) ;; >
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (> (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 11) ;; <= 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (<= (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 12) ;; >= 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (>= (cadr stack) (car stack)) (cddr stack)) dump)
                  (eq? instr 13) ;; length
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (length (car stack)) (cdr stack)) dump)
                  (eq? instr 14) ;; exact?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (exact? (car stack)) (cdr stack)) dump)
                  (eq? instr 15) ;; inexact?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (inexact? (car stack)) (cdr stack)) dump)
                  (eq? instr 16) ;; display
                    (begin
                        (display (car stack))
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons #v (cdr stack)) dump))
                  (eq? instr 18) ;; real?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (real? (car stack)) (cdr stack)) dump)
                  (eq? instr 19) ;; integer?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (integer? (car stack)) (cdr stack)) dump)
                  (eq? instr 20) ;; complex?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (complex? (car stack)) (cdr stack)) dump)
                  (eq? instr 21) ;; rational?
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (rational? (car stack)) (cdr stack)) dump)
                  (eq? instr 22) ;; gcd
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (gcd (car stack)) (cdr stack)) dump)
                  (eq? instr 23) ;; lcm
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (lcm (car stack)) (cdr stack)) dump)
                  (eq? instr 24) ;; numerator 
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (numerator (car stack)) (cdr stack)) dump)
                  (eq? instr 25) ;; denomenator
                        (hydra@vm code
                                  env
                                  (+ ip 1)
                                  (cons (denomenator (car stack)) (cdr stack)) dump)
                  (eq? instr 26) ;; = 
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (= (car stack) (cadr stack)) (cddr stack)) dump)
                  (eq? instr 27) ;; eq?
                        (hydra@vm code
                                 env
                                 (+ ip 1)
                                 (cons (eq? (car stack) (cadr stack)) (cddr stack)) dump)
                  (eq? instr 28) ;; jump
                        (hydra@vm code
                                 env
                                 (+ ip (hydra@operand c))
                                 stack dump)
                  (eq? instr 29) ;; cmp
                        (if (car stack) ;; if the top of the stack is true
                            (hydra@vm code env (+ ip 1) (cdr stack) dump) ;; jump to the <then> portion
                            (hydra@vm code env (+ ip (hydra@operand c)) (cdr stack) dump))
                  (eq? instr 30) ;; call
                        (if (and (not (null? stack)) (hydra@lambda? (car stack)))
                            ;; create a list from the current registers, cons this to dump, and 
                            ;; recurse over hydra@vm. 
                            ;; need to support CALLing primitives too, since they could be passed
                            ;; in to HOFs...
                            (let ((env-and-stack (build-environment (nth (cadar stack) 0) (cdr stack) (nth (cadar stack) 2))))
                                (hydra@vm
                                    (nth (cadar stack) 1)
                                    (car env-and-stack)
                                    0 '() 
                                    (cons (list code env ip (cadr env-and-stack)) dump)))
                            (begin
                                (display "in <else> of CALL\n")
                                #f))
                  (eq? instr 31) ;; environment-load; there is never a raw #f, so this is safe
                        (with r (hydra@lookup (hydra@operand c) env)
                            (if (eq? r #f)
                                #f
                                (hydra@vm
                                    code 
                                    env
                                    (+ ip 1) 
                                    (cons r stack)
                                    dump)))
                  (eq? instr 32) ;; tail-call 
                        (if (and (not (null? stack)) (eq? (caar stack) 'compiled-lambda))
                            (hydra@vm
                                (nth (cdar stack) 0)
                                (nth (cdar stack) 1)
                                0 '() 
                                dump)
                            #f)
                  (eq? instr 33) ;; %define
                        (begin
                            (hydra@add-env! (car stack) (cadr stack) env)
                            (hydra@vm
                                code env (+ ip 1)
                                (cons #v stack)
                                dump))
                  (eq? instr 34) ;; %set!
                        (begin
                            (hydra@set-env! (car stack) (cadr stack) env)
                            (hydra@vm
                                code env (+ ip 1)
                                (cons #v stack)
                                dump))))))


; syntax to make the above nicer:
; (define-instruction := "numeq" '() '() (+ ip 1) (cons (= (car stack) (cadr stack)) (cddr stack)))
; (define-instruction '() "jump" '() '() '() (if (car stack) ...))
; this should also fill in the top-level environment for those that have non-null name
; (define-instruction name short-description code-manip stack-manip IP-manip resulting code) 

; (primitive 0) can still be defeated, I think:
; (define foo '(primitive 0))
; (foo '(1 2 3 4))
; Definitely can be defeated. Really, need to move to some SRFI-9-ish system that
; users cannot create their own versions of.
(define *tlenv* '({
    :car (primitive . 0) ;; (primitive . 0) 
    :cdr (primitive . 1)
    :cons (primitive . 2)
    ;; 3 is load a value onto the stack
    ;; 4 is push a nil onto the stack
    :%- (primitive . 5) ;; primitive math operations with arity 2
    :%+ (primitive . 6)
    :%* (primitive . 7)
    :%/ (primitive . 8)
    :%< (primitive . 9)
    :%> (primitive . 10)
    :%<= (primitive . 11)
    :%>= (primitive . 12)
    :if (syntax . primitive-syntax-if)
    :fn (syntax . primitive-syntax-fn)
    :lambda (syntax . primitive-syntax-fn)
    :quote (syntax . primitive-syntax-quote)
    :quasi-quote (syntax . primitive-syntax-qquote)
    :unquote (syntax . primitve-syntax-unquote)
    :unquote-splice (syntax . primitive-syntax-unqsplice)
    :length (primitive . 13)
    :exact? (primitive . 14)
    :inexact? (primitive . 15)
    :display (primitive . 16)
    :apply (primitive . 17)
    :real? (primitive . 18)
    :integer? (primitive . 19)
    :complex? (primitive . 20)
    :rational? (primitive . 21)
    :gcd (primitive . 22)
    :lcm (primitive . 23)
    :numerator (primitive . 24)
    :denomenator (primitive . 25)
    :%= (primitive . 26) ;; probably has to place the value on stack rather than #t, #f for failure
    :eq? (primitive . 27)
    ;; 28 is jump
    ;; 29 is compare
    ;; 30 is call
    ;; 31 is environment-load
    ;; 32 is tail-call
    :+ (syntax . primitive-syntax-plus) ;; variable arity syntax
    :- (syntax . primitive-syntax-minus)
    :* (syntax . primitive-syntax-mult)
    :/ (syntax . primitive-syntax-div)
    :< (syntax . primitive-syntax-lt)
    :> (syntax . primitive-syntax-gt)
    :<= (syntax . primitive-syntax-lte)
    :>= (syntax . primitive-syntax-gte)
    := (syntax . primitive-syntax-numeq)
    :define (syntax . primitive-syntax-define)
    :set! (syntax . primitive-syntax-set)
    :define-syntax (syntax . primitive-syntax-defsyn)
    :define-macro (syntax . primitive-syntax-defmac)
    :%define (primitive . 33)
    :%set! (primitive . 34)
}))

(define (hydra@lookup item env)
    " look up item in the current environment, returning #f for not found"
    (cond
        (not (symbol? item)) item ;; to support ((fn (x) (+ x x)) (+ x x) 3)
        (null? env) #f
        (dict-has? (car env) item) (nth (car env) item)
        else (hydra@lookup item (cdr env))))

(define (compile-lambda rst env)
    (list 'compiled-lambda
        (vector
            (list-copy env)
            (append-map
                (fn (x) (hydra@eval x env))
                (cdr rst))
            (car rst)))) 

(define (hydra@lambda? x)
    (and (pair? x) (eq? (car x) 'compiled-lambda)))

(define (hydra@primitive? x)
    (and (pair? x) (eq? (car x) 'primitive)))

(define (hydra@syntax? x)
    (and (pair? x) (eq? (car x) 'syntax)))

(define (hydra@add-env! name value environment)
    " adds name to the environment, but also returns
      (load #v), so that the compiler adds the correct
      value (this is in the semantics of Vesta, so I thought
      it should be left in Hydra as well)"
    (cset! (car environment) name value))

(define (hydra@set-env! name value environment)
    " sets a value in the current environment, and returns
      an error if that binding has not been previously defined"
    (cond
        (null? environment) (error (format "SET! error: undefined name \"~a\"" name))
        (dict-has? (car environment) name)
            (cset! (car environment) name value)
        else hydra@set-env! name value (cdr environment)))

(define (reverse-append x)
    "append but in reverse"
    (cond
        (null? x) x
        (null? (cdr x)) (car x)
        else (append (reverse-append (cddr x)) (cadr x) (car x))))

(define (show x) (display "show: ") (display x) (newline) x)

(define (hydra@eval line env (thusfar '()))
    (if (null? line)
        thusfar
        (cond
            (vector? line) (list (list 3 line))
            (dict? line) (list (list 3 line) )
            (symbol? line) (list (list 31 line)) ;; environment-load
            (pair? line) 
                (let* ((fst (car line)) ;; decompose line into first & rest
                       (v (hydra@lookup fst env)) ;; find fst in env
                       (rst (cdr line))) 
                   (if (eq? fst #f) ;; failed to find fst
                       (error (format "Symbol not found: ~a~%" fst)) 
                       (cond 
                            (hydra@syntax? v) ;; primitive syntax
                                (cond
                                    (eq? (cdr v) 'primitive-syntax-quote)
                                        (if (null? (car rst))
                                            '((4))
                                            (list (list 3 (car rst))))
                                    (eq? (cdr v) 'primitive-syntax-plus)
                                        (append 
                                            '((3 0))
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%+ env))))))
                                                rst))
                                    (eq? (cdr v) 'primitive-syntax-minus)
                                        (cond
                                            (= (length rst) 1)
                                                (append '((3 0))
                                                    (hydra@eval (car rst) env)
                                                    (list (list (hydra@lookup '%- env))))
                                            (> (length rst) 1)
                                                (append 
                                                    (hydra@eval (car rst) env)
                                                    (append-map
                                                        (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%- env))))))
                                                        (cdr rst)))
                                            else (error "minus fail"))
                                    (eq? (cdr v) 'primitive-syntax-mult)
                                        (append 
                                            '((3 1))
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%* env))))))
                                                rst))
                                    (eq? (cdr v) 'primitive-syntax-div)
                                        (cond
                                            (= (length rst) 1)
                                                (append '((3 1))
                                                    (hydra@eval (car rst) env)
                                                    (list (list (hydra@lookup '%/ env))))
                                            (> (length rst) 1)
                                                (append 
                                                    (hydra@eval (car rst) env)
                                                    (append-map
                                                        (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%/ env))))))
                                                        (cdr rst)))
                                            else (error "division fail"))
                                    (eq? (cdr v) 'primitive-syntax-numeq)
                                        (cond
                                            (= (length rst) 1)
                                                (list (list 3 #t))
                                            (> (length rst) 1)
                                                (append
                                                    (hydra@eval (car rst) env)
                                                    (append-map
                                                        (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%= env))))))
                                                        (cdr rst)))
                                            else (error "numeq fail"))
                                    (eq? (cdr v) 'primitive-syntax-define)
                                        (let ((name (car rst))
                                               (value (cadr rst)))
                                            (cond
                                                (pair? name) 
                                                    (append
                                                        (hydra@eval (cons 'fn (cons (cdar rst) (cdr rst))) env)
                                                        (list (list 3 (caar rst)))
                                                        (list (list (cdr (hydra@lookup '%define env))))) 
                                                (symbol? name)
                                                    (append
                                                        (hydra@eval value env)
                                                        (list (list 3 name))
                                                        (list (list (cdr (hydra@lookup '%define env)))))
                                                else (error "DEFINE error: define SYMBOL VALUE | DEFINE PAIR S-EXPR*")))
                                    (eq? (cdr v) 'primitive-syntax-set)
                                        (let ((name (car rst))
                                              (value (cadr rst)))
                                           (if (symbol? name) 
                                                (append
                                                    (hydra@eval value env)
                                                    (list (list 3 name))
                                                    (list (list (cdr (hydra@lookup '%set! env)))))
                                                (error "SET!: set! SYMBOL S-EXPR*")))
                                    (eq? (cdr v) 'primitive-syntax-defsyn)
                                        #t
                                    (eq? (cdr v) 'primitive-syntax-defmac)
                                        #t
                                    (eq? (cdr v) 'primitive-syntax-fn)
                                        (list (list 3 ;; load
                                            (compile-lambda rst env)))
                                    (eq? (cdr v) 'primitive-syntax-lt)
                                        (append 
                                            (hydra@eval (car rst) env)
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%< env))))))
                                                (cdr rst)))
                                    (eq? (cdr v) 'primitive-syntax-gt)
                                        (append 
                                            (hydra@eval (car rst) env)
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%> env))))))
                                                (cdr rst)))
                                    (eq? (cdr v) 'primitive-syntax-lte)
                                        (append 
                                            (hydra@eval (car rst) env)
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%<= env))))))
                                                (cdr rst)))
                                    (eq? (cdr v) 'primitive-syntax-gte)
                                        (append 
                                            (hydra@eval (car rst) env)
                                            (append-map
                                                (fn (x) (append (hydra@eval x env) (list (list (cdr (hydra@lookup '%>= env))))))
                                                (cdr rst)))
                                    (eq? (cdr v) 'primitive-syntax-if)
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
                            (pair? fst) ;; fst is a pair, so we just blindly attempt to compile it. May cause an error that has to be caught in CALL. some lifting might fix this...
                                (append (reverse-append
                                            (map (fn (x) (hydra@eval x env)) rst))
                                        (hydra@eval fst env)
                                        (list (list 30)))
                            (hydra@primitive? v) ;; primitive procedure
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
                                    (list (list (cdr v))))
                            (hydra@lambda? v) ;; hydra closure
                                (append (reverse-append
                                            (map (fn (x) (hydra@eval x env)) rst))
                                            (list (list 3 v))
                                            (list (list 30)))
                            (symbol? fst) ;; fst is a symbol, but it has no mapping in our current env; write to environment-load
                                (append (reverse-append
                                            (map (fn (x) (hydra@eval x env)) rst))
                                            (list (list 31 fst))
                                            (list (list 30)))
                            else (error "error: the only applicable types are primitive procedures, closures & syntax"))))

            else (list (list 3 line)))))

;; need to separate user values from what 
;; is returned in the eval...
(define (top-level-print x)
    " print #<foo> at the top level"
    (cond
        (hydra@lambda? x) (display "#<closure>")
        (hydra@primitive? x) (display (format "#<primitive-procedure ~a>" (cdr x)))
        (hydra@syntax? x) (display (format "#<syntax ~a>" (cdr x)))
        else (display x)))

(define (hydra@load src-file env)
    "an implementation of the primitive procedure load"
    (with f (open src-file :read)
        (with-exception-handler
            (fn (x) (display (format "An error occured while loading ~S: ~a\n" src-file x)) (close f))
            (fn ()
                (letrec ((loop (fn (expr)
                                    (if (eq? expr #e)
                                        #v
                                        (begin
                                            (hydra@vm (list->vector (hydra@eval expr env)) env)
                                            (loop (read f)))))))
                    (loop (read f)))
                (close f)))))
                                    
(define (hydra@repl)
    (display "h; ")
    (with inp (read)
     (if (and (eq? (type inp) "Pair") (eq? (car inp) 'unquote))
        (cond
         (eq? (cadr inp) 'exit) (quit)
         (eq? (cadr inp) 'q) (quit)
         (eq? (cadr inp) 'quit) (quit)
         (eq? (cadr inp) 'dribble) (begin (hydra@repl))
         (eq? (cadr inp) 'save) (begin (hydra@repl))
         (eq? (cadr inp) 'save-and-die) (begin (hydra@repl))
         else (begin (display (format "Unknown command: ~a~%" (cadr inp))) (hydra@repl)))
        (if (not (pair? inp))
            (if (eq? inp #v)
                (hydra@repl)
                (begin
                    (top-level-print (hydra@lookup inp *tlenv*))
                    (newline)
                    (hydra@repl)))
            (with r (hydra@vm (list->vector (hydra@eval inp *tlenv*)) *tlenv*)
                (if (eq? r #v)
                 (hydra@repl)
                 (begin
                    (top-level-print r)
                    (newline)
                    (hydra@repl))))))))

(define (hydra@main)
    (display "\n\t()\n\t  ()\n\t()  ()\nDigamma/Hydra: 2012.0/r0\n")
    (hydra@repl))    
