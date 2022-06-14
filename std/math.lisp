; UTILITIES

; (starcall * [10 20]) -> (* 10 20)

; FUNCTIONS
(=> factorial (/ * (range 1 (+ _ 1))))
(=> sgn (? _ (? (> _ 0) 1 -1) 0))
(=> #/ (// (float (starcall + _)) (# _)))
(=> max_2 [a b] (? (> a b) a b))
(=> min_2 [a b] (? (< a b) a b))
(=> max (/ max_2 _))
(=> min (/ min_2 _))
(=> abs (* _ (sgn _)))
(=> ** [base exp] (/ * (repeat exp base) 1))
(=> >> [i offset] (// i (** 2 offset)))
(=> << [i offset] (* i (** 2 offset)))

(= avg #/)
(= pow **)
(= shfr >>)
(= shfl <<)
