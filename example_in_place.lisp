(=> square (* _ _))

; Demonstrates usage of in-place operators.
(= a 10)
(*= a 5) ; 10 * 5 = 50
(*= a 2 3) ; 50 * 2 * 3 = 300

(= numbers [10 20 30])

; In-place map.
(|> numbers square) ; -> [100 200 300]
(|> numbers neg) ; -> [-100 -200 -300]
