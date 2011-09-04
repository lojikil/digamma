; a simple JSON parser that returns Digamma objects based on the source. 
; Should provide File & string based versions, but currently is String only.
; File based should be easy enough.
; zlib/png licensed & (c) 2010 Stefan Edwards
;

(def (json-string s delim offset)
     "Parse a JSON string, breaking on delim. Delim should be either
      #\" or #\'."
      (cond
        (>= offset (length s)) (error "JSON Parse Error: stream ends before string")
        (eq? (nth s offset) delim) '()
        (eq? (nth s offset) #\\) 
            (if (<= (+ offset 1) (length s)) ; should add cases for \n, \r, &c.
              (cons (nth s (+ offset 1))
                    (json-string s delim (+ offset 2)))
              (error "JSON Parse Error: Stream ends before string"))
        else (cons (nth s offset) (json-string s delim (+ offset 1)))))

; probably would be better as a lookup table:
; table => [#\t #\r #\u #\e '$] and state would be index into this table

(def (json-true s offset (state 0)) 
     "Parse JSON true literal"
     (if 
       (> offset (length s)) (error "JSON Parse Error: stream ends before literal")
       (cond
         (and (eq? state 0) (eq? (nth s offset) #\t)) (json-true s (+ offset 1) 1)
         (and (eq? state 1) (eq? (nth s offset) #\r)) (json-true s (+ offset 1) 2)
         (and (eq? state 2) (eq? (nth s offset) #\u)) (json-true s (+ offset 1) 3)
         (and (eq? state 3) (eq? (nth s offset) #\e)) (json-true s (+ offset 1) 4)
         (eq? state 4)
            (cond
              (>= offset (length s)) #t
              (eq? (nth s offset) #\}) #t
              (eq? (nth s offset) #\,) #t
              (eq? (nth s offset) #\space) #t
              else (error "JSON Parse Error: unknown literal in true")))))

(def (json-false s offset (state 0))
     "Parse JSON false literal"
     (if 
       (>= offset (length s)) (error "JSON Parse Error: stream ends before literal")
       (cond
         (and (eq? state 0) (eq? (nth s offset) #\f)) (json-false s (+ offset 1) 1)
         (and (eq? state 1) (eq? (nth s offset) #\a)) (json-false s (+ offset 1) 2)
         (and (eq? state 2) (eq? (nth s offset) #\l)) (json-false s (+ offset 1) 3)
         (and (eq? state 3) (eq? (nth s offset) #\s)) (json-false s (+ offset 1) 4)
         (and (eq? state 4) (eq? (nth s offset) #\e)) (json-false s (+ offset 1) 5)
         (eq? state 5)
            (cond
              (>= offset (length s)) #t
              (eq? (nth s (+ offset 1)) #\}) #t
              (eq? (nth s (+ offset 1)) #\,) #t
              (eq? (nth s (+ offset 1)) #\space) #t
              else (error "JSON Parse Error: unknown literal in false")))))

(def (json-null s offset (state 0))
     "Parse JSON null literal"
     (if 
       (>= offset (length s)) (error "JSON Parse Error: stream ends before literal")
       (cond
         (and (eq? state 0) (eq? (nth s offset) #\n)) (json-null s (+ offset 1) 1)
         (and (eq? state 1) (eq? (nth s offset) #\u)) (json-null s (+ offset 1) 2)
         (and (eq? state 2) (eq? (nth s offset) #\l)) (json-null s (+ offset 1) 3)
         (and (eq? state 3) (eq? (nth s offset) #\l)) (json-null s (+ offset 1) 4)
         (eq? state 4)
            (cond
              (>= offset (length s)) '()
              (eq? (nth s offset) #\}) '()
              (eq? (nth s offset) #\,) '()
              (eq? (nth s offset) #\space) '()
              else (error "JSON Parse Error: unknown literal in null")))))

(def (json-number s offset (state 0))
     "Parse JSON number"
     #f)

(def (next-token s offset)
     (if (>= offset (length s))
       #e
       (cond
         (eq? (nth s offset) #\{) 'OCURLY
         (eq? (nth s offset) #\}) 'CCURLY
         (eq? (nth s offset) #\[) 'OVECTOR
         (eq? (nth s offset) #\]) 'CVECTOR
         (eq? (nth s offset) #\') (json-string s #\' (+ offset 1))
         (eq? (nth s offset) #\") (json-string s #\" (+ offset 1))
         (eq? (nth s offset) #\:) 'PAIRSEP
         (eq? (nth s offset) #\,) 'MEMBERSEP
         (numeric? (nth s offset)) (json-number s offset)
         (eq? (nth s offset) #\t) (json-true s offset 1) ;; skip to second state
         (eq? (nth s offset) #\f) (json-false s offset 1)
         (eq? (nth s offset) #\n) (json-null s offset 1)
         else (error "JSON Parsing Failed: invalid JSON object"))))

(def (json-object s offset)
     " returns a JSON object from the current stream "
     #f)

(def (json-array s offset)
     " returns a JSON array from the current stream "
     (let ((value (next-token s offset)))
       (cond
         (eq? value 'OVECTOR) (json-array s (+ offset 1))
         (eq? value 'OCCURLY) (json-objet s (+ offset 1))
         else #f)))

; string->json should just call various intenral functions:
; parse object, array, number, literal, string
; should return whatever object is applicable:
; "true" => #t
; "nil" => '()
; "[1,2,3,4]" => [1 2 3 4]
; "{"test" : [1,2], "stuff" : [3,4]}" => { :test [1 2] :stuff [3 4]}
; "1" => 1
; "{\"test\"}" => Error
; "{1}" => Error
; &c. Vesta doesn't yet support Unicode, so that's one minor draw back.
; have to fix unicode support at some point.
;

(def (json->value s)
     " Convert a JSON string to a Digamma value"
     {})

(def (value->json s)
     " Serialize a Digamma object to JSON"
     #f)
