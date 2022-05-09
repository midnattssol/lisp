(use! "math")

; Short-circuit nothing.
(=> ?? {_ default} (? (== _ Nothing) default _))

(=> is_type [a typename]
    (typematch [[(type typename)]] [a])
)

(=> always_iterable (
    if!
        (is_type _ "list")
        _
        [_]
))

(=> ,, [l r] (, (always_iterable l) (always_iterable r)))

(=> zip [a b] (do
    (= out [])
    (= i 0)
    (= l (min_2 (# a) (# b)))

    (while! (< i l) (do
        (push out [(@ i a) (@ i b)])
        (++ i)
    ))

    out
))

(=> filter [f a] (do
    (= out [])
    (= i 0)
    (= l (# a))
    (while! (< i l) (do
        (if! (f (@ i a)) (push out (@ i a)) Nothing)
        (++ i)
    ))
    out
))
