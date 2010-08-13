(define-syntax sif ((_ <test> <then>) (cond (<test> <then>))) ((_ <test> <then> <else>) (cond (<test> <then>) (else <else>))))
(sif (< 3 4) (display "Yes"))
(sif (< 3 4) (display "Yes") (display "no"))
