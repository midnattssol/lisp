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

(=> emirp? (&& (prime? _) (prime? (parse (reverse! (repr _))))))
(. #[putl! _] (/? emirp? 2:1001))
