(load '../logic/unify.ss)

(define *initial-environment*
    '((nth int (list (? a)) => (? a))
      (car (pair (? a)) => (? a))
      (cdr (pair (? a) => (pair (? a))))
      (cons (? a) (? b) => (pair (? a) (? b)))
      ;; these are poly morphic, but ad-hocly so, but this
      ;; type signature seems to specify that they aren't.
      ;; Need to specify a notation of Union types.
      ;; num => Union(int real rational complex)
      ;; seq => Union(pair string vector)
      ;; col => Union(pair string vector dict)
      ;; bool => #t | #f
      ;; goal => #s | #u
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
