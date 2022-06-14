(use! "functional")


; Use an indexer to compress an iterable.
; For example, (compress [1 0 1] [10 20 30]) -> [10 30].
(=> compress [indexer iterable] (do
    (= out l[])
    (assert (== (# iterable) (# indexer)))
    (where! [iterable item index] (@ index indexer) (= out (linsert item -1 out)))
    out
))

; Find the first index at which (== item (fn (@ i iter))).
(=> find_where [fn iter item] (do
    (= continue Yes)
    (= i 0)
    (= n (# iter))
    (while! continue
        (&= continue (!= (fn (@ i iter)) item))
        (++ i)
        (&= continue (< i n))
    )
    (? (!= i n) (-- i) Nil)
))

; Take from an iterable until the closure returns False.
(=> take_while [fn iter] (do
    (= i (coalesce (find_where fn iter No) -1))
    (-- i)
    (if! i (slice iter 0 i) [])
))

; Take from an iterable from the point that the closure returns False.
(=> drop_while [fn iter] (do
    (= i (coalesce (find_where fn iter No) -1))
    (if! (> i 0) (slice iter i) [])
))

(= result (compress [Yes No Yes] [10 20 30]))
(putl! result)
(= result (map + (ı 20 70) (ı 50)))
(putl! result)
(= a (map (-> (% _ 7)) 4:20))
(putl! a)
(putl! (take_while bool a))
(putl! (drop_while bool a))
