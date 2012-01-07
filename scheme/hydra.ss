;; A simple stack-based VM for Digamma
;; Meant to support both interpretation in Vesta as
;; well as compilation with E'
;; copyright 2011 Stefan Edwards; please see the LICENSE
;; file for details

(define (vm@eval code env (stack '()))
     (if (null? code)
         (car stack)
         (let* ((c (car code))
                (instr (instruction c))
                (oper  (operand c))) ;; this is simple to transition to register vm
              (cond ;; case would make a lot of sense here...
                  (eq? instr 0) ;; car
                        (vm@eval (cdr code)
                                 env
                                 (cons (car-op (car stack)) (cdr stack)))
                  (eq? instr 1) ;; cdr
                        (vm@eval (cdr code)
                                 env
                                 (cons (cdr-op (car stack)) (cdr stack)))
                  (eq? instr 2) ;; cons
                        (vm@eval (cdr code)
                                 env
                                 (cons (cons-op (car stack)
                                                (cadr stack))
                                       (cdr stack)))
                  (eq? instr 3) ;; load
                        (vm@eval (cdr code)
                                 env
                                 (cons oper stack))))))

(define *tlenv* {})

(define (hydra@eval line env)
    #f)

(def hydra@repl (fn ()
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
            (hydra@repl))))))

(define (hydra@main)
    (display "\n\t()\n\t  ()\n\t()  ()\nDigamma/Hydra: 2009.3/r0\n")
    (hydra@repl))    
