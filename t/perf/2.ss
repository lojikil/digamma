(define (fib n) (cond ((<= n 1) 1) (else (+ (fib (- n 1)) (fib (- n 2))))))
(display (fib 22))
(newline)
(exit)
