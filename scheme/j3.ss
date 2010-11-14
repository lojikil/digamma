(def nfib (fn (i j n)
	(if (<= n 0)
		i
		(nfib (+ i j) i (- n 1)))))
