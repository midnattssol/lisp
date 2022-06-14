; Not yet implemented
(=> fib [0] 0)
(=> fib [1] 0)
(=> fib [_] (+ (fib (-- _)) (fib (-- _))))


; ; Should compile to something similar to this:
; (=> metafib [_] (do
;     (if! (== _ 0) (return 0))
;     (if! (== _ 1) (return 0))
;     (+ (fib (-- _)) (fib (-- _)))
; ))
