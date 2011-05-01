; just a simple test to keep around for testing E' with
; you can safely ignore this :D
(def fib (fn (i j n)
	(if (<= n 0)
		i	
		(fib (+ i j) i (- n 1)))))
