#!/usr/bin/env vesta
(load 'hydra.ss)
;; Vesta has *command-line* defined even if it isn't being run in script
;; mode, so I think Hydra should follow suit
(hydra@add-env! '*command-line* '() *tlenv*)
(if (> (length *command-line*) 0)
    (begin
        (hydra@set-env! '*command-line* *command-line* *tlenv*)
        (hydra@load (nth *command-line* 0) *tlenv*))
        (hydra@main))
