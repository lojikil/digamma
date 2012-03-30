(define (ip->hex ip)
    (with res (map (fn (x) (coerce x 'int)) (string-split-charset ip "."))
          (+
            (<< (& (nth res 0) 255) 24)
            (<< (& (nth res 1) 255) 16)
            (<< (& (nth res 2) 255) 8)
            (& (nth res 3) 255))))

(define (hex->ip hexip)
    (format "~n.~n.~n.~n"
        (>> (& hexip #xFF000000) 24)
        (>> (& hexip #x00FF0000) 16)
        (>> (& hexip #x0000FF00) 8)
        (& hexip #x000000FF)))

(define (bitvec->hint x (idx 0))
    (if (>= idx (length x))
        0     
        (+ (<< (if (nth x idx) 1 0) (- 31 idx)) (bitvec->hint x (+ idx 1)))))

(define (gen-netmask l)
    (bitvec->hint (make-vector l #t)))

(define (useable-ips cidr-len)
    (- (exp2 (- 32 cidr-len)) 2))

(define (cidr->ip-list baseip mask)
    (define (foobar b o l)
        (if (< o l)
         (cons (hex->ip (+ b o)) (foobar b (+ o 1) l))
         '()))
    (foobar (+ (ip->hex baseip) 1) 0 (useable-ips mask)))

(define (range-stream start end)
    (with state start
        (lambda ()
            (if (< state end)
                (begin
                    (set! state (+ state 1))
                    (- state 1))
                (error "stream end")))))

;; causes a crash; look into this
(define (map-stream p r)
    (let ((v '()))
        (with-exception-handler
            (fn (c)
               v)
            (letrec ((inner-map-stream (lambda () (set! v (append v (list (p (r))))) (inner-map-stream))))
                inner-map-stream))))
                

(define (better-cidr->ip-list baseip mask)
    (with b (ip->hex baseip)
        (map-stream
            (fn (x) (hex->ip (+ b x)))
            (range-stream 0 (+ (useable-ips mask) 1)))))

(define (cidr->ip-stream baseip mask)
    (with b (ip->hex baseip)
        (map-stream
            (fn (x) (hex->ip (+ b x)))
            (range-stream 0 (+ (useable-ips mask) 1)))))
