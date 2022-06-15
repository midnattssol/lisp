(use! "itertools")

; https://code.golf/lucky-numbers

; Generate the _ first lucky numbers.
(=> lucky (do
    (= seq 1:_:2)
    (= i 1)

    (while! (> (# seq) i) (do
        (= s (range (# seq)))
        (.= s #[% (+ _ 1) (@ i seq)])
        (.@= seq s)
        (++ i)
    ))

    seq
))

(. #[putl! _] (lucky 1000))
