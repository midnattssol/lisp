(=> mod10 (% _ 100))

(= x (rand 20))
(|> x mod10)
(put x)
