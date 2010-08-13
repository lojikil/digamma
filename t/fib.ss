(def fib (fn (n) (cond ((< n 2) 1) (else (+ (fib (- n 1)) (fib (- n 2)))))))

(display (fib 20))
(newline)
