(=> mod10 (% _ 10))

; Seeds the RNG.
(seed 15)

(= x (rand 20))
(|> x mod10)
(put x) ; [4 1 6 3 5 6 6 3 1 1 9 0 9 3 5 9 3 9 2 4]
