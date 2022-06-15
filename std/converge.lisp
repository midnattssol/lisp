(=> converge [fn initial] (do
    (= old initial)
    (loop! (do
        (= new (fn old))
        (if! (== old new) (return new))
        (= old new)
    ))
))

(=> converge-accumulate [fn initial] (do
    (= old initial)
    (= out [])
    (loop! (do
        (= new (fn old))
        (,= out [old])
        (if! (== old new) (return out))
        (= old new)
    ))
))
