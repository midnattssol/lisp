(=> mod10 (% _ 10))

(= x (rand 20))
(|> x mod10)
(put x)
