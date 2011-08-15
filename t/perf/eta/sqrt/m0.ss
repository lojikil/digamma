(define (sqrt-iter guess x)
  (if (good-enough? guess x)
      guess
      (sqrt-iter (improve guess x)
                 x)))

(define (improve guess x)
  (average guess (/ x guess)))

(define (average x y)
  (/ (+ x y) 2))

(define (good-enough? guess x)
  (< (abs (- (* guess guess) x)) 0.001))

(define (mysqrt x)
  (sqrt-iter 1.0 x))

(define (int x acc step)
  (if (>= x 10000.0)
      acc
      (int (+ x step)
           (+ acc (* step (mysqrt x)))
           step)))

(write (int 0.0 0.0 .001))
(newline)
(exit)
