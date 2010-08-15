; attempt at colour version
;(load "./prelude.ss")

; to do:
;  - angled rectangle
;  - add flood fill colour to rectangle & circle
;  - text rendering
;  - predefined chart rendering (ala gnuplot)

(define-macro with (var val :body b)
	(list (cons fn (cons (cons var '()) b)) val))
(def foreach-proc (fn (proc col)
	(if (empty? col)
		#v	
		(begin 
			(proc (first col))
			(foreach-proc proc (rest col))))))
(def step-range (fn (start end step)
      (if (< step 0)
              (if (> end start)
				'()
				(cons start (step-range (+ start step) end step)))
			  (if (> start end)
				'()
				(cons start (step-range (+ start step) end step))))))
(def plot-fixed (fn (i x y x-size y-size offset colour)
	;(display (format "plot-fixed x: ~a, y: ~a~%" (truncate x) (truncate y)))
	;(interrogate)
	(if (or (< x 0) (>= x x-size) (< y 0) (>= y y-size))
		#f
		(begin
			(cset! i (* offset(+ (truncate x) (* (truncate y) x-size))) (nth colour 0))
			(cset! i (+ 1 (* offset (+ (truncate x) (* (truncate y) x-size)))) (nth colour 1))
			(cset! i (+ 2 (* offset (+ (truncate x) (* (truncate y) x-size)))) (nth colour 2))))))
(def plot-fixed-line (fn (i x0 y0 x1 y1 x-size y-size colour)
	;(display (format "Made it to plot-fixed-line(in t0m) ~%"))
	(if (= x0 x1)
		(foreach-proc (fn (y) (plot-fixed i x0 y x-size y-size 3 colour)) (step-range y0 y1 1))
		((fn (m) 
			(display (format "Made it to inside with...~%"))
			(foreach-proc (fn (x) (plot-fixed i x (+ (* m (- x x1)) y1) x-size y-size 3 colour)) (step-range x0 x1 1 ))) (/ (- y1 y0) (- x1 x0))))))
(def plot-fixed-circle (fn (i x y r x-size y-size offset colour)
        (foreach-proc (fn (theta)
                (let ((xp (* r (abs (cos theta)))) (yp (* r (abs (sin theta)))))
                  (display (format "xp => ~n ; yp => ~n ~%" xp yp))
                  (plot-fixed canvas (truncate (+ x xp)) (truncate (+ y yp)) x-size y-size offset colour)
                  (plot-fixed canvas (truncate (- x xp)) (truncate (- y yp)) x-size y-size offset colour)
                  (plot-fixed canvas (truncate (+ x xp)) (truncate (- y yp)) x-size y-size offset colour)
                  (plot-fixed canvas (truncate (- x xp)) (truncate (+ y yp)) x-size y-size offset colour)))
                (step-range 0.0 90.0 0.1))))
(def plot-fixed-rectangle (fn (i x0 y0 w h x-size y-size colour)
	(plot-fixed-line i x0 y0 (+ x0 w) y0 x-size y-size colour)
	(plot-fixed-line i (+ x0 w) y0 (+ x0 w) (+ y0 h) x-size y-size colour) ; working...
	(plot-fixed-line i x0 (+ y0 h) (+ x0 w) (+ y0 h) x-size y-size colour)
	(plot-fixed-line i x0 y0 x0 (+ y0 h) x-size y-size colour)))
; points is a vector or list of lists (x y) 
; this draws lines between (x[n], y[n]) -> (x[n + 1], y[n + 1])
(def plot-fixed-polygon (fn (i points x-size y-size colour)
	(def pfp-int (fn (i points x-size y-size initial colour) 
		(if (> (length points) 1)
		  '())))))
