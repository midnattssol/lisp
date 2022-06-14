(use! "functional")


; Use an indexer to compress an iterable.
; For example, (compress [1 0 1] [10 20 30]) -> [10 30].
(=> compress [indexer iterable] (do
    (= out [])
    (assert (== (# iterable) (# indexer)))
    (where! [iterable item index] (@ index indexer) (= out (push! item out)))
    out
))


(= .@ compress)

; Find the first index at which (== item (fn (@ i iter))).
(=> find_where [fn iter item] (do
    (= i 0)
    (= n (# iter))
    (while! (< i n)
        (if! (== (bool (fn (@ i iter))) item) (return i))
        (++ i)
    )
    Nil
))


; Take from an iterable until the closure returns False.
(=> take_while [fn iter] (do
    (= i (default_on_nil (find_where fn iter No) 0))
    (-- i)
    (if! i (slice iter 0 i) [])
))

; Take from an iterable from the point that the closure returns False.
(=> drop_while [fn iter] (do
    (= i (default_on_nil (find_where fn iter No) -1))
    (if! (> i 0) (slice iter i) [])
))
