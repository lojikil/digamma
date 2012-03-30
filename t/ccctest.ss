(define f #f)
(+ 1 (call/cc (lambda (k) (newline) (newline) (display "K --> ") (display k) (newline) (newline) (set! f k) 1)))
