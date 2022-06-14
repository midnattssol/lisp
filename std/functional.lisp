(use! "math")

; Short-circuit Nil.
(=> coalesce [_ default] (? (== _ Nil) default _))
(=> ?> [fn _] (unless! (== _ Nil) (fn _)))
(= ?? coalesce)

(=> is_type? [a typename]
    (typematch [[(type typename)]] [a])
)

(=> always_iterable (
    if! (is_type? _ "list") _ [_]
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
        (if! (f (@ i a)) (push out (@ i a)))
        (++ i)
    ))
    out
))


(= /? filter)

(=> compose [fn0 fn1] #[fn0 (fn1 _)])
(=> after [fn0 fn1] #[fn1 (fn0 _)])
(= <.. compose)
(= ..> after)
