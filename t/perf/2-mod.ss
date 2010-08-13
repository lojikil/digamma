; modified to fit the current stackless, which does not load init.ss from
; anywhere, and hence does not have define or exit
(def fib (fn (n) (cond ((<= n 1) 1) (else (+ (fib (- n 1)) (fib (- n 2)))))))
(display (fib 22))
(newline)
(quit)
