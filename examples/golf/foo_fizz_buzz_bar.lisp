(use! "itertools")

; https://code.golf/foo-fizz-buzz-bar
(=> foo_fizz_buzz_bar [x] (do
    (= words ["Foo" "Fizz" "Buzz" "Bar"])
    (= nums [2 3 5 7])
    (.= nums #[! (% x _)])
    (. put (if!
        (/ || nums)
        (.@ nums words)
        [x]
    ))
    (put "\n")
))


(. foo_fizz_buzz_bar 1:1001)
