(define (myeven? n)
  (if (= n 0)
    #t
    (if (< n 0)
      #f
      (myodd? (- n 1)))))

(define (myodd? n)
  (if (= n 0)
    #f
    (if (< n 0)
      #t
      (myeven? (- n 1)))))

(display "2? ")
(display (myeven? 2))
(display " ")
(display (myodd? 2))
(newline)

(display "3? ")
(display (myeven? 3))
(display " ")
(display (myodd? 3))
(newline)

(display "212457? ")
(display (myeven? 212457))
(display " ")
(display (myodd? 212457))
(newline)
