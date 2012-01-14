(load 'hydra.ss)
(display (hydra@eval '(car (cdr (cons 1 (cons 2 '())))) *tlenv*))
