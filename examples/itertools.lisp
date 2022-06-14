(use! "itertools")

(seed 15)
(=> coinflip (% (head! (rand 1)) 2))

; Randomly choose numbers to filter out.
(putl! (/? coinflip :30)) ; [1 3 4 7 8 9 10 12 13 14 15 16 17 21 23 24 25 26 28]

; Compress a list.
(= result (.@ [Yes No Yes] [10 20 30]))
(putl! result) ; [10 30]

; Take and drop while a condition is met.
(= nums 4:20)

(putl! nums) ; [4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19]
(putl! (take_while #[% _ 7] nums)) ; [4 5 6]
(putl! (drop_while #[% _ 7] nums)) ; [7 8 9 10 11 12 13 14 15 16 17 18 19]
