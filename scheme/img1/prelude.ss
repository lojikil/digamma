; A simple collexion of useful utilities, that may over lap with init.ss, but
; to be used at my discresion...
;
; Scheme is like a ball of snow. You can add any amount of snow to it and it still looks like a ball of snow.
; Moreover, snow is cleaner than mud.
; -- Marc Feeley
(define-macro defn (name formals :body body)
	(list 'def name (cons 'fn (cons formals body))))
(define-macro define (name :body body)
	(if (eq? (type name) "Pair")
		(list 'def (car name) (cons fn (cons (cdr name) body)))
		(list 'def name (car body))))
(def null? (fn (n) (eq? n '())))
(def pair? (fn (n) (eq? (type n) "Pair")))
(def eof-object? (fn (n) (eq? n #e)))
(def ncons (fn (x) (cons x '())))
(def unzip (fn (ll)
	(def inner-u-r (fn (xl yl sl)
		(if (null? sl)
			(list xl yl)
			(inner-u-r (append xl (ncons (car (car sl)))) (append yl (ncons (car (cdr (car sl))))) (cdr sl)))))
	(inner-u-r '() '() ll)))
(def zip (fn (xs ys)
	(if (null? xs)
		'()
		(cons (cons (car xs) (cons (car ys) '())) (zip (cdr xs) (cdr ys))))))
		
(define-macro and (<item> :rest <body>) 
	(if (null? <body>) 
		<item>
		(list 'if <item> (cons 'and <body>))))
(define-macro or (<item> :rest <body>) 
	(if (null? <body>)
		<item>
		(list 'if <item> #t (cons 'or <body>))))
(def endswith? (fn (ending str)
	(eq? (cslice str (- 0 (length ending)) -1) ending)))
(def beginswith? (fn (b str)
	(eq? (cslice str 0 (length b)) b)))
(define-macro with (name value :body body) (list (cons 'fn (cons (cons name '()) body)) value))
(define-macro let (bind :body letbody)
	((fn (unbounds) (cons (cons 'fn (cons (car unbounds) letbody)) (car (cdr unbounds)))) (unzip bind)))
(define-macro aif (<if-test> <if-true> :opt <if-false>) #f)

(defn but-last (lst)
	"remove everything but the last element"
	(if (null? (cdr lst))
		'()
		(cons (car lst) (but-last (cdr lst)))))
(define-macro cond (:body body)
	(if (null? body) 
		#f
		(if (pair? (car body))
			(if (eq? (car (car body)) 'else)
				(cdr (car body))
				(list 'if (car (car body)) (cons 'begin (cdr (car body))) (list 'cond (cdr body))))
			(error "Malformed if block"))))
(defn match (str pat) #f)
(defn split (str pat) #f)
(defn subst (str pat) #f)
(defn subst! (str pat) #f)
(defn map (proc col)
	(if (empty? col)
		col
		(ccons (proc (first col)) (map proc (rest col)))))
(defn map-apply (proc col)
	(if (empty? col)
		col
		(ccons (apply proc (first col)) (map-apply proc (rest col)))))
