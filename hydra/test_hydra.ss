(load 'hydra.ss)
(define code '(car (cdr (cons 1 (cons 2 '())))))
(define hlap (hydra@eval code *tlenv*))
(display "original source code: ")
(write code)
(newline) 
(display "HLAP output: ")
(display hlap)
(newline)
(display (vm@eval hlap *tlenv*))
(newline)

;; To Do:
;; tests for:
;;  - basic structures
;;  - basic values
;;  - syntax: if, define, set!
;;  - primitives: car/cdr/cons/&c.
;;  - lambdas
;;  - HOFs
;;  - SRFIs
;;  - Datalog support
;;  - Define-syntax
;;  - Same should be done for Eprime...

;; Hand code the HLAP here, and run tests.
