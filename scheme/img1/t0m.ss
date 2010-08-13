(load "./img-test.ss")
(load "./prelude.ss")
(define-macro with (var val :body b)
	(list (cons fn (cons (cons var '()) b)) val))
(def map (fn (proc col)
	(if (empty? col)
		col
		(ccons (proc (first col)) (map proc (rest col))))))
(def foreach-proc (fn (proc col)
	(if (empty? col)
		#v	
		(begin 
			(proc (first col))
			(foreach-proc proc (rest col))))))
(def step-range (fn (start end step)
	;(display (format "Within step-range~%"))
	(if (> start end)
		'()
		(cons start (step-range (+ start step) end step)))))
(def plot-fixed-line (fn (i x0 y0 x1 y1)
	(display (format "Made it to plot-fixed-line(in t0m) ~%"))
	(if (= x0 x1) ; Windows seem to close around if blocks; the x0 variable isn't available below, it seems...
		#f
		((fn (m) 
			(display (format "Made it to inside with...~%"))
			(foreach-proc (fn (x) (plot-fixed i x (+ (* m (- x x1)) y1))) (step-range x0 x1 1))) (/ (- y1 y0) (- x1 x0))))))
