; from: http://en.wikipedia.org/wiki/Unification_(computer_science)

;In Prolog:
;1. A variable which is uninstantiated—i.e. no previous unifications were
;   performed on it—can be unified with an atom, a term, or another unins-
;   tantiated variable, thus effectively becoming its alias. In many
;   modern Prolog dialects and in first-order logic, a variable cannot
;   be unified with a term that contains it; this is the so called
;   occurs check.
;2. Two atoms can only be unified if they are identical.
;3. Similarly, a term can be unified with another term if the top funct-
;   ion symbols and arities of the terms are identical and if the
;   parameters can be unified simultaneously. Note that this is a
;   recursive behavior.

(define (var? x)
    (and (pair? x) (eq? (car x) '?)))

(define (unify o0 o1i env)
    (cond
        (and (var? o0) (var? o1))
            (if (or
                    (not (eq? (assq env o0) '()))
                    (not (eq? (assq env o1) '())))
                #s
                #u)
        (eq? (type o0) (type o1)) #s
        (var? o0) o1
        (var? o1) o0
        else
            (if (= (length o0) (length o1))
                kkkkkkkkk
        
