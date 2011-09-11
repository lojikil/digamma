; simple Eprime-able implementation of syntax-rules
; nicer than the one written in C

(def (match-pattern P F)
     #f)
(def (rewrite-pattern env template)
     " rewrite a template based on the environment.
       Doesn't do '...' matching yet, obviously "
     (cond
       (null? template) '()
       (pair? template) (cons (rewrite-pattern env (car template))
                              (rewrite-pattern env (cdr template)))
       (symbol? template)
           (if (has-key? env template)
               (nth env template)
               template)
       else template))

(def (syntax-expand literals rules)
     #f)
