(def make-bag make-dict)
(def bag-add! (fn (x y)
	(if (dict-has? x y)
			(cset! x y (+ (nth x y) 1))
			(cset! x y 1))))
(def bag-members keys) ; wrong, needs to display each member multiple time
(def bag-cardinal (fn (x) (length (keys x)))) ; wrong, needs count each member + the number of times it occurs
