(def *site-lib* '("." "./lib" "~/.digamma/lib")) ; for use in require
(def tilde-expand (fn (st)
	(string-append (sys-getenv "HOME") (cslice st 1 -1))))
(def find-requirements (fn (lib-path lib)
	(if (null? lib-path)
		#f
		#t)))
(def require (fn (lib)
	(if (endswith? lib ".ss")
		(find-requirements *site-lib* lib)
		(find-requirements *site-lib* (string-append lib ".ss")))))