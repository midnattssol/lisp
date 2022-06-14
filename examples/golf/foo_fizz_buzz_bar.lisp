(use! "itertools")

; https://code.golf/foo-fizz-buzz-bar
(=> foo_fizz_buzz_bar (do
    (= nums [2 3 5 7])
    (= words ["Foo" "Fizz" "Buzz" "Bar"])
    (.= nums (-> [n] (! (% _ n))))
    (. put (if!
        (/ | nums)
        (compress nums words)
        [_]
    ))
    (put "\n")
))


(. foo_fizz_buzz_bar 1:1001)
