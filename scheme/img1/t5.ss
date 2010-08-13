(load "t0m.ss")
; a few notes:
; 0 - if the plot point is negative, have to actually add *more* to the 
;     value
; 1 - Wondering if I should plot things with the negative side towards
;     the top of the image, and flip the image before printing...

; this assumes a relative scale of x belongs to [-1,1], y belongs to [-1,1]
; have to work on better code for this, but for now it's fine.
(def tnc (fn (n)
	(truncate (- 150 (* n 150)))))
(def plot-relative (fn (img x y)
	(plot-fixed img (truncate (- 150 (* x 150))) (truncate (- 150 (* y 150))))))
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
(plot-fixed-line img 220 75 230 85)
(plot-fixed-line img 70 170 230 170)
(plot-fixed-line img 70 170 230 180)
(plot-relative img -0.9 0.1)
(plot-relative img -0.8 0.2)
(plot-relative img -0.7 0.3)
(plot-relative img -0.6 0.4)
(plot-relative img -0.5 0.5)
(plot-relative img -0.4 0.6)
(plot-relative img -0.3 0.7)
(plot-relative img -0.2 0.8)
(plot-relative img -0.1 0.9)
(plot-relative img 0.9 0.1)
(plot-relative img 0.8 0.2)
(plot-relative img 0.7 0.3)
(plot-relative img 0.6 0.4)
(plot-relative img 0.5 0.5)
(plot-relative img 0.4 0.6)
(plot-relative img 0.3 0.7)
(plot-relative img 0.2 0.8)
(plot-relative img 0.1 0.9)
(display (format "Attemping to plot -0.9, -0.1:-0.1, -0.9 (~n,~n:~n,~n)~%" (tnc -0.9) (tnc -0.1) (tnc -0.1) (tnc -0.9)))
(plot-relative-line img -0.1 -0.9 -0.9 -0.1)
;(display (format "Attemping to plot 0.1,-0.9:0.9,-0.1 (~n,~n:~n,~n)~%" (tnc 0.1) (tnc -0.9) (tnc 0.9) (tnc -0.1)))
;(plot-relative-line img 0.9 -0.1 0.1 -0.9)
; attempt to plot a relative point
(def fil (open "test5.pnm" :write))
(display (format "P1~%300 300~%~a" img) fil)
(close fil)
