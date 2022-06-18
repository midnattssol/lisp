(use! "math")

; Short-circuit Nil.
(=> nil? (== _ Nil))
(=> default_on_nil [_ x] (? (nil? _) x _))
(=> ?> [fn _] (unless! (nil? _) (fn _)))

(=> is_type? [a typename]
    (typematch [[(type typename)]] [a])
)

(=> always_iterable (
    if! (is_type? _ "list") _ [_]
))

(=> ,, [l r] (, (always_iterable l) (always_iterable r)))


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
