; Simple VM machine for Digamma
; supports the same spec as Vesta, but compiles code to a Virtual Machine
; first (with possible JIT'ing later). Meant to be used with E', but can 
; be tested with Vesta. 

; To Use: 
; compile ceres from Vesta via E', and use ceres@main from within a C
; file. Once this is closer to being finished, I'll make a write up.
; As time goes on, I may want to implement more of this within Digamma
; itself, as I maybe able to improve some of the run time functions too.
;
;

(def (compile-code code)
     (cond
       (literal? code) (list 'literal code)
       (vector? code) #f))

;; how to determine if something has been compiled before?
;; probably don't care; Just compile whatever is written at 
;; the top level.
(define (ceres@eval code) 
   (let* ((vm-code (ceres@compile code))
          (inst (instruction vm-code)))
        (cond 
            (eq? inst 0) 
