(use! "functional")

; https://code.golf/emirp-numbers

; Gets whether or not a number is prime.
(=> prime? (do
    (= i 2)
    (while!
        (leq (* i i) _)
        (if! (% _ i) (++ i) (return No))
    )
    Yes
))

(=> emirp? (&& (prime? _) (prime? (parse (rev! (repr _))))))
(. #[putl! _] (/? emirp? 2:1001))
