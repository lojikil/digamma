;; this is a simple test of the new (def (foo bar) ...) 
;; support in lift-lambda/lift-tail-lambda
;; the resutling code should be exactly the same as the
;; output from j1.ss

(def (fib i j n)
	(if (<= n 0)
		i	
		(fib (+ i j) i (- n 1))))
(def (scheme-main)
    (display (fib 0 1 10))
    (newline)
    (display (fib 0 1 20))
    (newline))
