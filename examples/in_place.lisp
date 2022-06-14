; Demonstrates usage of in-place operators.
(= a 10)
(*= a 5) ; 10 * 5 = 50
(*= a 2 3) ; 50 * 2 * 3 = 300

(putl! a)

(= numbers [1 2 3])

; In-place map.
(.= numbers #[* _ _]) ; -> [1 4 9]
(.= numbers neg) ; -> [-1 -4 -9]
(.= numbers #[// _ 4.]) ; -> [-0.25 -1 -2.25]
(putl! numbers)
