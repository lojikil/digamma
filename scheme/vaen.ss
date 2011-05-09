;; Simple logic system for generating truth tables & truth-function analysis
(def *logo* "\tVAEN  V\n\t   N  A\n\tVAEN  E\n\t   N\n\tVAEN  N\n")
(def *release* "09-MAY-2011")

(def vaen_main (fn ()
    (display *logo*)
    (display (format "vaen version ~s~%" *release*))))
