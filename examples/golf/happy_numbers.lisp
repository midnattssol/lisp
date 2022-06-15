(use! "functional")

; https://code.golf/happy-numbers

; Count occurences in an iterable.
(=> count [x it] (# (/? #[== _ x] it)))

; Squares and sums all digits, for example
; 19 -> 1 * 1 + 9 * 9 -> 82.
(=> sum_of_squares (do
    (= digits ($ (split "" (repr _)) 1))
    (.= digits parse)
    (/ + (. #[* _ _] digits))
))

; Check if a number is happy or not.
(=> happy? (do
    (= out [_])
    (while!
        (== (count (tail! out) out) 1)
        (,= out [(sum_of_squares (tail! out))])
    )
    (== (tail! out) 1)
))

(. #[putl! _] (/? happy? 1:201))
