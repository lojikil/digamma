;; this is a test of lift-tail-lambda's support of multiple
;; body members

(def (fib i j n)
    (display (format "i = ~a, j = ~a, n = ~a~%" i j n))
	(if (<= n 0)
		i	
		(fib (+ i j) i (- n 1))))
(def (scheme-main)
    (display (fib 0 1 10))
    (newline)
    (display (fib 0 1 20))
    (newline))
