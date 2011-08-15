(define (range start end)
  (if (= start end)
    '()
    (cons start (range (+ start 1) end))))

(map (fn (x) (* x x)) (range 0 1000))
