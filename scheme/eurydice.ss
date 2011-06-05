;; Digamma pretty printer
;; Should also be able to translate between (def foo (fn (x) ...))
;; and (def (foo x) ...)
;; Also, it would be nice if it could do some sort of lint-style checks

(def *indent-size* 4)
(def *line-per-arg* #t)
(def *line-after-docstring* #t)

(def eurydice (fn (fin fout)
    #t))
