(use! "functional")

(=> random_half [args] (% (@ 0 (rand 1)) 2))

; Randomly choose numbers to filter out.
(put (filter random_half :30))
