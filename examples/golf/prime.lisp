(use! "functional")

; Gets whether or not a number is prime.
(=> prime? (do
    (= i 2)
    (while!
        (leq (* i i) _)
        (unless! (% _ i) (return No) (++ i))
    )
    Yes
))

(. #[putl! _] (/? emirp? 2:101))
