; singlular string rip : rips a string on a single character
(define (ssr str chr idx hold)
	(cond
		((empty? str) #f)
		((= (length str) idx) (ncons (str hold idx)))
		((char=? (str idx) chr) (cons (str hold idx) (ssr str chr (+ idx 1) (+ idx 1))))
		(else (ssr str chr (+ idx 1) hold))))
(display 1)
(newline)
; multiple string rip : rips a string on multiple characters
(define (check-set set char)
	(cond
		((empty? set) #f)
		((char=? (first set) char) #t)
		(else (check-set (rest set) char))))
(define (msr str chr idx hold)
	(cond
		((empty? str) #f)
		((= (length str) idx) (ncons (str hold idx)))
		((check-set chr (str idx)) (cons (str hold idx) (msr str chr (+ idx 1) (+ idx 1))))
		(else (msr str chr (+ idx 1) hold))))

; and now, for the tie-in...
(define (str-sep str sep)
  (cond
   ((char? sep) (ssr str sep 0 0))
   ((collection? sep) (msr str sep 0 0))
   (else #f)))
(define (str-join strings joiner)
  (apply string-append (list-join strings joiner)))
(define (str-contains? src fnd idx fidx)
        (cond
                ((= idx (length src)) (if (= fidx (length fnd)) #t #f))
                ((= fidx (length fnd)) #t)
                ((char=? (src idx) (fnd fidx)) (str-contains? src fnd (+ idx 1) (+ fidx 1)))
                (else (str-contains? src fnd (+ idx 1) 0))))
;need to return the start & end indexes...
(define (str-find src fnd idx fidx)
        (cond
                ((= idx (length src)) (if (= fidx (length fnd)) idx #f))
                ((= fidx (length fnd)) idx)
                ((char=? (src idx) (fnd fidx)) (str-find src fnd (+ idx 1) (+ fidx 1)))
                (else (str-find src fnd (+ idx 1) 0))))

