; a Bigloo version of the fib test
(module foo
    (main main))
(define (fib i j n)
	(if (<= n 0)
		i	
		(fib (+ i j) i (- n 1))))
(define (main) 
    (display (fib 0 1 10))
    (newline)
    (display (fib 0 1 20))
    (newline)
    (display (fib 0 1 30))
    (newline)
    (display (fib 0 1 32))
    (newline))
(main)
