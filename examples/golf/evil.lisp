(use! "math")
(use! "itertools")

; Handles numbers up to 64 (5 bits).
(=> evil? [x] (! (% (/ + (. #[bool (& x (<< 1 _))] :6)) 2)))
(. #[putl! _] (/? evil? :51))
