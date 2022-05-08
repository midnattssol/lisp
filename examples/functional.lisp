; Short-circuit nothing.
(=> ?? {_ default} (? (== _ Nothing) default _))

(=> is_type [a typename] (do
    (typematch [[(type typename)]] [a])
))

; (=> paramorphism [])
(=> always_iterable (
    if!
        (is_type _ "list")
        _
        [_]
))

(=> ,, [l r] (, (always_iterable l) (always_iterable r)))

(= x 0)


; Stolen from math
(=> min_2 {a b} (? (< a b) a b))
(=> min (fold min_2 _))

(=> zip [a b] (do
    (= out [])
    (= i 0)
    (= l (min_2 (# a) (# b)))

    (while! (< i l) (do
        (= out (insert [(@ i a) (@ i b)] -1 out))
        (++ i)
    ))

    out
))

(=> filter [f a] (do
    (= out [])
    (= i 0)
    (= l (# a))
    (while! (< i l) (do
        (if! (f (@ i a)) (= out (insert (@ i a) -1 out)) Nothing)
        (++ i)
    ))
    out
))

(=> ! (? _ 0 1))

(=> chooser [args] (% (@ 0 (rand 1)) 2))

; (put (zip 1:5 10:17))

(put (filter chooser :30))

; (while expression expression)

; [10 20 30] [1 2 3]
; [] [1 1]
