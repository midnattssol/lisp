(use! "functional")

; https://code.golf/abundant-numbers
(=> proper_divisors [x] (/? #[! (% x _)] 1:x))
(=> abundant? (> (/ + (proper_divisors _) 0) _))
(. #[putl! _] (/? abundant? 1:201))
