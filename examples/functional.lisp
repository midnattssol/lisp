(use! "functional")

(seed 15)
(=> coinflip (% (head! (rand 1)) 2))

; Randomly choose numbers to filter out.
(= numbers :30)
(/?= numbers coinflip)
(putl! numbers) ; -> [1 3 4 7 8 9 10 12 13 14 15 16 17 21 23 24 25 26 28] 
