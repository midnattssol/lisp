; https://code.golf/collatz
(=> collatz (do
    (if! (== _ 1) (return [_]))
    (= x (? (% _ 2) (+ 1 (* 3 _)) (// _ 2)))
    (push! _ (collatz x))
))

; Recursively get the stopping time.
(=> stopping_time (do
    (if! (== _ 1) (return 0))
    (= next (? (% _ 2) (+ 1 (* 3 _)) (// _ 2)))
    (+ 1 (stopping_time next))
))

(. #[putl! (stopping_time _)] 1:1001)
