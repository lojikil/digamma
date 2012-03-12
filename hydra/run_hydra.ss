#!/usr/bin/env vesta
(load 'hydra.ss)
;; Vesta has *command-line* defined even if it isn't being run in script
;; mode, so I think Hydra should follow suit
(hydra@add-env! '*command-line* '())
(if (> (length *command-line*) 0)
    (begin
        (hydra@set-env! '*command-line* *command-line*)
        (hydra@load (nth *command-line* 0)))
        (hydra@main))
