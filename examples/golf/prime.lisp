(use! "functional")

; https://code.golf/prime-numbers

; Gets whether or not a number is prime.
(=> prime? (do
    (= i 2)
    (while!
        (leq (* i i) _)
        (if! (% _ i) (++ i) (return No))
    )
    Yes
))

(. #[putl! _] (/? prime? 2:101))
