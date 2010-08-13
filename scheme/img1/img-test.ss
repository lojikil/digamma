(def draw-x-axis (fn (i x y)
	(if (= x 300)
		#t
		(begin
			(cset! i (+ x (* y 300)) 1)
			(draw-x-axis i (+ x 1) y)))))
(def draw-y-axis (fn (i x y)
	(if (= y 300)
		#t
		(begin
			(cset! i (+ x (* y 300)) 1)
			(draw-y-axis i x (+ y 1))))))
(def plot-fixed (fn (i x y)
	(display (format "plot-fixed x: ~a, y: ~a~%" x y))
	(if (or (< x 0) (> x 300) (< y 0) (> y 300))
		#f
		(cset! i (+ (truncate x) (* (truncate y) 300)) 1))))

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
