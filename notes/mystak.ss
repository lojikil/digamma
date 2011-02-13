; vim:setf digamma: 
; Pystache session:
;>>> import pystache
;>>> pystache.render('Hello {{    person}}!',{'person':'You'})
;u'Hello You!'
;>>> pystache.render('Hello {{    d d d}}!',{'d d d':'You'})
;u'Hello You!'
;>>> pystache.render('Hello {{    d d d}}!',{'ddd': 'You'})
;'Hello !'

; doing this with string ripping would be much better, but I'll have to add
; another parameter (like last-rip) to mystak-sm, so that we rip up to offset - 2.
; this will reduce the number of intermediary little strings created, and speed up
; the processor overall.
; Also:
; a {{    d d d    }} b
; should match a context with 'd d d' defined (i.e. trim right hand side of strings)
; a {{ d d d
; should return:
; a {{ d d d
; thinking about it, should probably go with a parser, rather than a simple
; state machine; this SM version though could be repurposed for SimpleTemplate:
; a $b c $d

(defn white-space? (c)
 (or (eq? c #\space) (eq? c #\newline) (eq? c #\tab) (eq? c #\vtab) (eq? c #\linefeed)))
(defn mystak-testing (template context)
    (defn mystak-sm (template context :opt (offset 0) :opt (state 0) :opt (tok '()))
        (cond
                (>= offset (length template)) '()
                (= state 0) ; should do this with rips, rather than multiple little strings...
                    (if (eq? (nth template offset) #\{)
                     (mystak-sm template context (+ offset 1) 1)
                     (cons (string (nth template offset)) (mystak-sm template context (+ offset 1) 0))) 
                (= state 1) ; made it to {, find another {, then a name & }}
                    (if (eq? (nth template offset) #\{)
                     (mystak-sm template context (+ offset 1) 2)
                     (cons (string #\{ (nth tempalte offset)) (mystak-sm template context (+ offset 1) 0))) 
                (= state 2) ; start collecting name, eat white
                    (if (white-space? (nth template offset))
                     (mystak-sm template context (+ offset 1) 2)
                     (mystak-sm template context offset 3))
                (= state 3) ; looking for }
                    (if (eq? (nth template offset) #\})
                     (mystak-sm template context (+ offset 1) 4)
                     (mystak-sm template context (+ offset 1) 3))
                (= state 4) ; found second }, now rewrite
                    (if (eq? (nth template offset) #\})
                     #f ; need to make this join tok, trim it, lookup the result in context & append it here
                     (mystk-sm template context offset 3))))
    (apply string-append (mystak-sm template context)))
;;;; Demo ;;;;
;(mystak-testing "<doc><name>{{name}}</name><location>{{location}}</location></doc>" { name: "testing-mystak" location: "/path"})
