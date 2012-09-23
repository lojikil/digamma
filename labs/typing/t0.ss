(load '../logic/unify.ss)

(define *initial-environment*
    '((nth seq int => (? a))
      (car (pair (? a)) => (? a))
      (cdr (pair (? a) => (pair (? a))))
      (cons (? a) (? b) => (pair (? a) (? b)))
      ;; these are poly morphic, but ad-hocly so, but this
      ;; type signature seems to specify that they aren't.
      ;; Need to specify a notation of Union types.
      ;; num => Union(int real rational complex)
      ;; seq => Union((pair (? a)) (string char) (vector (? a)))
      ;; col => Union((pair (? a)) (string char) (vector (? a)) dict)
      ;; bool => #t | #f
      ;; goal => #s | #u
      ;; union types can be represented by prolog-OR...
      ;; basically, this would need unify (from logic), every & any
      ;; from Kanren.
      (+ num ... => num) 
      (- num ... => num)
      (/ num ... => num)
      (* num ... => num)
      (> num ... => bool)
      (< num ... => bool)
      (>= num ... => bool)
      (<= num ... => bool)
      (!= num ... => bool)
      (length seq => int)))
