;; The begining of a string library
;; to be 100% honest, this actually would work with most types
;; of sequences, so long as the elements can be eq?'d effectively

(defn string-split-charset (str sepset :opt (start 0) :opt (offset 0))
	(defn sepchar? (c sepset :opt (so 0))
		(cond 
			(>= so (length sepset)) #f
			(eq? (nth sepset so) c) #t
			else (sepchar? c sepset (+ so 1))))
	(cond
		(>= offset (length str)) (cons (cslice str start offset) '())
		(sepchar? (nth str offset) sepset) 
			(cons 
				(cslice str start offset) 
				(string-split-charset str sepset (+ offset 1) (+ offset 1)))
		else (string-split-charset str sepset start (+ offset 1))))
(defn string-join (l i :opt (acc '()))
        (if (empty? (cdr l))
                (apply string-append (append acc (list (car l))))
                (string-join (cdr l) i (append acc (list (car l) i)))))
(defn string-reverse (s offset nu )
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
