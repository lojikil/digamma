; a simple JSON parser that returns Digamma objects based on the source. 
; Should provide File & string based versions, but currently is String only.
; File based should be easy enough.
; zlib/png licensed & (c) 2010 Stefan Edwards

(def parse-json (fn (s) #t))

; parse-json should just call various intenral functions:
; parse object, array, number, literal, string
; should return whatever object is applicable:
; "true" => #t
; "nil" => '()
; "[1,2,3,4]" => [1 2 3 4]
; "{"test" : [1,2], "stuff" : [3,4]}" => { :test [1 2] :stuff [3 4]}
; &c. Vesta doesn't yet support Unicode, so that's one minor draw back.
; have to fix unicode support at some point.
