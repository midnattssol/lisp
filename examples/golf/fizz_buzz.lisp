(use! "itertools")

; https://code.golf/fizz-buzz
(=> fizz_buzz [x] (do
    (= words ["Fizz" "Buzz"])
    (= nums [3 5])
    (.= nums #[! (% x _)])
    (. put (if!
        (/ | nums)
        (.@ nums words)
        [x]
    ))
    (put "\n")
))


(. fizz_buzz 1:1001)
