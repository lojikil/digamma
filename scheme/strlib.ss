;; The begining of a string library
;; to be 100% honest, this actually would work with most types
;; of sequences, so long as the elements can be eq?'d effectively

(def (in-string? c str (start 0))
      (if (>= start (length str))
        #f
        (if (eq? c (nth str start))
          #t
          (in-string? c str (+ start 1)))))

(def (string-split-charset str sepset (start 0) (offset 0))
	(cond
		(>= offset (length str)) (cons (cslice str start offset) '())
		(in-string? (nth str offset) sepset) 
			(cons 
				(cslice str start offset) 
				(string-split-charset str sepset (+ offset 1) (+ offset 1)))
		else (string-split-charset str sepset start (+ offset 1))))
(def (string-join l i (acc '()))
        (if (empty? (cdr l))
                (apply string-append (append acc (list (car l))))
                (string-join (cdr l) i (append acc (list (car l) i)))))
(def (string-reverse s offset nu )
	(cond 
		(< offset 0) 
			(begin 
				(display (format "nu == ~a\n" nu)) 
				(if (eq? nu '())
					(display "nu is null\n")
					(display "nu isn't null\n"))
				(list nu)) 
		(>= offset (length s)) (begin (display "in offset overflow\n") s)
		(eq? nu '()) 
			(begin
				(display "In nu init\n")
			(string-reverse s (- (length s) 1) (make-string (length s))))
		else
			(begin
				(display (format "In begin; offset == ~a; nu == ~a\n" offset nu))
				(cset! nu offset (nth s offset))
				(string-reverse s (- offset 1) nu))))
