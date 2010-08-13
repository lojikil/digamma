(load "./prelude.ss")
(def draw-x-axis (fn (i x y x-max y-size)
	(if (= x x-max)
		#t
		(begin
			(cset! i (+ x (* y y-size)) 1)
			(draw-x-axis i (+ x 1) y)))))
(def draw-y-axis (fn (i x y y-size)
	(if (= y y-size)
		#t
		(begin
			(cset! i (+ x (* y y-size)) 1)
			(draw-y-axis i x (+ y 1))))))
(def plot-fixed (fn (i x y x-size y-size)
	;(display (format "plot-fixed x: ~a, y: ~a~%" x y))
	(if (or (< x 0) (> x x-size) (< y 0) (> y y-size))
		#f
		(cset! i (+ (truncate x) (* (truncate y) y-size)) 1))))

;; Example usage:
; (def img (make-vector (* 300 300) 0))
; (draw-x-axis img 0 150)
; (draw-y-axis img 150 0)
;; Most likely can unify the two axis
;; drawing functions; also, they should
;; have bangs on their names, and should
;; return void when done.
; (draw-x-axis! img 0 150)
; (draw-y-axis! img 150 0)

(define-macro with (var val :body b)
	(list (cons fn (cons (cons var '()) b)) val))
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
(def plot-fixed-line (fn (i x0 y0 x1 y1 x-size y-size)
	;(display (format "Made it to plot-fixed-line(in t0m) ~%"))
	(if (= x0 x1) ; Windows seem to close around if blocks; the x0 variable isn't available below, it seems...
		#f
		((fn (m) 
			(display (format "Made it to inside with...~%"))
			(foreach-proc (fn (x) (plot-fixed i x (+ (* m (- x x1)) y1) x-size y-size)) (step-range x0 x1 1))) (/ (- y1 y0) (- x1 x0))))))

(def tnc (fn (n)
	(truncate (- 150 (* n 150)))))
(def xtrunc (fn (x x-origin)
	(if (>= x 0)
		(truncate (+ x-origin (* x x-origin)))
		(truncate (- x-origin (* (abs x) x-origin))))))
(def ytrunc (fn (y y-origin)
	(if (<= y 0)
		(truncate (+ y-origin (* (abs y) y-origin)))
		(truncate (- y-origin (* y y-origin))))))
(def plot-relative (fn (img x y)
	(plot-fixed img (xtrunc x) (ytrunc y 150))))
(def plot-relative-line (fn (img x0 y0 x1 y1)
	(plot-fixed-line img (truncate (- 150 (* x0 150)))
				(truncate (- 150 (* y0 150)))
				(truncate (- 150 (* x1 150)))
				(truncate (- 150 (* y1 150))))))
; need to make plot-fixed coerce things to
; int if possible.
(def img (make-vector (* 300 300) 0))
(draw-x-axis img 0 150)
(draw-y-axis img 150 0)
(display (format "Sine plot~%"))
(foreach-proc 
	(fn (x) 
		(display (format "~n : ~n (~n:~n)~%" x (sin x) (xtrunc x) (ytrunc (sin x))))(plot-relative img x (sin x))) 
	(step-range -1.0 1.0 0.1))
(display (format "Cosine plot~%"))
(foreach-proc 
	(fn (x) 
		(display (format "~n : ~n (~n:~n)~%" x (cos x) (xtrunc x) (ytrunc (cos x)))) (plot-relative img x (cos x))) 
	(step-range -1.0 1.0 0.1))
; attempt to plot a relative point
(def fil (open "test10.pnm" :write))
(display (format "P1~%300 300~%~a" img) fil)
(close fil)
