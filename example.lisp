; Assign thing.

; (= h {{n} (* (resolve n) 10)})
(=> square {n} (* (resolve n) (resolve n)))
; (put (resolve h))

(put (call (resolve square) 10))
; (put (call {{n} (* (resolve n) 10)} 12))

; Should work after the variables are properly implemented.
; (=> square {n} (* n n))
; (put (call square 10))
