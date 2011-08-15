(define (range start end)
  (if (= start end)
    '()
    (cons start (range (+ start 1) end))))

(map (lambda (x) (* x x)) (range 0 1000))
(exit)
