(=> square (* _ _))
(=> fourth (/ _ 4))

; Demonstrates usage of in-place operators.
(= a 10)
(*= a 5) ; 10 * 5 = 50
(*= a 2 3) ; 50 * 2 * 3 = 300

(= numbers [1 2 3])

; In-place map.
(|> numbers square) ; -> [1 4 9]
(|> numbers neg) ; -> [-1 -4 -9]
(|> numbers fourth) ; -> [-0.25 -1 -2.25]
