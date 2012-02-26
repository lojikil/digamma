(load 'hydra.ss)
(define code '(car (cdr (cons 1 (cons 2 '())))))
(define hlap (hydra@eval code *tlenv*))
(display "original source code: ")
(write code)
(newline) 
(display "HLAP output: ")
(display hlap)
(newline)
(display (hydra@vm hlap *tlenv*))
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

;; need macro/syntax:

;; (hydra-test digamma expected-hlap expected-value)

(define-macro hydra-test (digamma-code expected-hlap expected-value)
    (list 'with 'h (list 'hydra@eval digamma-code '*tlenv*)
        (list 'if (list 'not (list 'equal? 'h expected-hlap))
            (list 'display (list 'format "[-] HLAP generation failed for ~a; expected ~a, got ~a~%" digamma-code expected-hlap 'h))
            (list 'with 'e (list 'hydra@vm 'h '*tlenv*)
                (list 'if (list 'not (list 'equal? 'e expected-value))
                    (list 'display (list 'format "[-] Eval failed for ~a; expected ~a, got ~a~%" digamma-code expected-value 'e))
                    (list 'display (list 'format "[+] test passed for ~a~%" digamma-code)))))))

(hydra-test (cons 1 2) ((3 2) (3 1) (2)) (1 . 2))
(hydra-test (cons 1 (cons 2 '())) ((4) (3 2) (2) (3 1) (2)) (1 2))
(hydra-test (car (cons 1 2)) ((3 2) (3 1) (2) (0)) 1)
(hydra-test (cdr (cons 1 2)) ((3 2) (3 1) (2) (1)) 2)
(hydra-test #t ((3 #t)) #t)
(hydra-test '(1 2 3) ((3 (1 2 3))) (1 2 3))
(hydra-test "test" ((3 "test")) "test")
(hydra-test (+ 1 2) ((3 2) (3 1) (6)) 3)
(hydra-test (+ 1 2 3 4) ((3 4) (3 3) (6) (3 2) (6) (3 1) (6)) 10)
(hydra-test (- 1 2) ((3 2) (3 1) (5)) -1)
(hydra-test (- 1 2 3 4) ((3 4) (3 3) (5) (3 2) (5) (3 1) (5)) -8)
(hydra-test (* 1 2) ((3 2) (3 1) (7)) 2)
(hydra-test (* 1 2 3 4) ((3 4) (3 3) (8) (3 2) (8) (3 1) (8)) 24)
(hydra-test (/ 1 2) ((3 2) (3 1) (9)) 1/2)
(hydra-test (/ 1 2 3 4) ((3 4) (3 3) (9) (3 2) (9) (3 1) (9)) 1/24)
