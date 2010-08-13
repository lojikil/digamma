(load "t0m.ss")
; need to make plot-fixed coerce things to
; int if possible.
(def img (make-vector (* 300 300) 0))
(draw-x-axis img 0 150)
(draw-y-axis img 150 0)
(plot-fixed-line img 220 75 230 85)
(plot-fixed-line img 70 170 230 170)
(plot-fixed-line img 70 170 230 180)
(def fil (open "test2.pnm" :write))
(display (format "P1~%300 300~%~a" img) fil)
(close fil)
