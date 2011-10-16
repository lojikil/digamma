(define-syntax fcond (else)
    ((_ else e) e)
    ((_ e1 e2) (if e1 e2 #f))
    ((_ e1 e2 e3 ...) (if e1 e2 (fcond e3 ...))))

(display "\nTest 0\n")
(fcond
  (eq? 3 3) (display "Yes!")
  else (display "no!"))
(newline)

(display "\nTest 1\n")
(fcond 
  (= 3 4) (display "Yes?")
  (= 4 4) (display "Yes!")
  else (display "No?"))

(newline)

(display "\nTest 2\n")
(fcond
  (= 3 4) (display "Yes?")
  (= 5 4) (display "Uh-oh")
  (= 6 4) (display "hmm")
  else (display "Yay!"))

(newline)
(exit)
