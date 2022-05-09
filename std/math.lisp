; UTILITIES

; (starcall * [10 20]) -> (* 10 20)
(=> starcall {fn vec} (eval (insert fn 0 vec)))
; Write on a new line.
(=> putl
    (noop
        (put _)
        (put "\n")
    )
)

; FUNCTIONS
(=> factorial (starcall * (range 1 (+ _ 1))))
(=> sgn (? _ (? (> _ 0) 1 -1) 0))
(=> ! (? _ 0 1))
(=> #/ (/ (starcall + _) (# _)))
(=> max_2 {a b} (? (> a b) a b))
(=> min_2 {a b} (? (< a b) a b))
(=> max (fold max_2 _))
(=> min (fold min_2 _))
(=> abs (* _ (sgn _)))
(=> ** {base exp} (starcall * (repeat exp base)))
(=> >> {i offset} (/ i (** 2 offset)))
(=> << {i offset} (* i (** 2 offset)))

(= avg #/)
(= pow **)
(= shfr >>)
(= shfl <<)
