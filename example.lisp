; comment world
; (put (list 'All good!' == 'hi') )
; (put (typematch (list (typeof 10)) (list 10)))
; (put (typematch (list (list (type '?'))) (list list)))
; (put (expression (== 10 20 a)))
; (= h (# 22:30))

; (put (repeat 10 20))
(put 10)
; (put (eval (insert (repeat 10 10) 0 +)))

; (put (call {{n} (* (resolve n) 10)} 1))
; (= add10 {{n} (+ (resolve n) 10)})
; (= add10 {{n} (+)})
; (put (call {{n} (+ (resolve n) 10)} 10))
; (put (call (resolve add10) 2))
; (put (call {{n} (+  10)} 5))
; (put (
;     call
;     {{n} (+ 10 20 (* 10 2 (resolve n)))}
;     2
; ))
; (put (call (resolve add10) 2))

; (put (call {{n m} (add 1 (resolve n))} 10 20))

; (def add10 {{n} {+ n 10}})

; (put (resolve h))
