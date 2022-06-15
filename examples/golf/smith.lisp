(use! "functional")
; https://code.golf/smith-numbers

; Get prime factors of a number.
(=> factors (do
    (= i 2)
    (while!
        (leq (* i i) _)
        (unless! (% _ i) (return (push! i (factors (// _ i)))) (++ i))
    )
    [_]
))

(. #[putl! _] (/? #[== (/ + (factors _)) _] 4:10_001))
