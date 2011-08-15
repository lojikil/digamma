(def (good-enough? guess x)
  (< (abs (- (* guess guess) x)) 0.001))

(def (average x y)
  (/ (+ x y) 2))

(def (improve guess x)
  (average guess (/ x guess)))

(def (sqrt-iter guess x)
  (if (good-enough? guess x)
      guess
      (sqrt-iter (improve guess x)
                 x)))

(def (mysqrt x)
  (sqrt-iter 1.0 x))

(def (integ x acc step)
  (if (>= x 10000.0)
      acc
      (integ (+ x step)
           (+ acc (* step (mysqrt x)))
           step)))

(def (scheme_main)
    (write (integ 0.0 0.0 .001))
    (newline))
