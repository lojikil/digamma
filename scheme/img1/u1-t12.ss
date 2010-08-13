(load "./t12.ss")
(def img (make-vector (* 1000 600) 0))
;(draw-y-axis img 150 0 600)
;(draw-x-axis img 0 300 300 600)
;(draw-x-axis img 0 300 300 600)
(plot-fixed-line img 0 300 1000 300 1000 600) ; draw x-axis
(plot-fixed-line img 500 0 500 600 1000 600)  ; draw y-axis
(plot-fixed-line img 0 600 1000 0 1000 600)
(foreach-proc 
	(fn (p) (apply plot-fixed-rectangle p))
	(list (list img 0 150 100 25 1000 600)
	  (list img 100 550 100 25 1000 600)
	  (list img 200 150 100 25 1000 600)
	  (list img 300 550 100 25 1000 600)
	  (list img 400 150 100 25 1000 600)
	  (list img 500 550 100 25 1000 600)))
(def fil (open "u1.pnm" :write))
(display (format "P1~%1000 600~%~a" img) fil)
(close fil)