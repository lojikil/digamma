(load "t0m.ss")
; a few notes:
; 0 - if the plot point is negative, have to actually add *more* to the 
;     value
; 1 - Wondering if I should plot things with the negative side towards
;     the top of the image, and flip the image before printing...
(def plot-relative (fn (x y min-x max-x min-y max-y)
	#v))
; need to make plot-fixed coerce things to
; int if possible.
(def img (make-vector (* 300 300) 0))
(draw-x-axis img 0 150)
(draw-y-axis img 150 0)
(plot-fixed-line img 220 75 230 85)
(plot-fixed-line img 70 170 230 170)
(plot-fixed-line img 70 170 230 180)
(plot-fixed img 120 210)
(plot-fixed img 121 210)
(plot-fixed img 120 209)
(plot-fixed img 121 209)
(plot-fixed img 120 90)
(plot-fixed img 121 90)
(plot-fixed img 120 89)
(plot-fixed img 121 89)
(plot-fixed img 120 45)
(plot-fixed img 121 45)
(plot-fixed img 120 44)
(plot-fixed img 121 44)
; attempt to plot a relative point
(def fil (open "test4.pnm" :write))
(display (format "P1~%300 300~%~a" img) fil)
(close fil)
