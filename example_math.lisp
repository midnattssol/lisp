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

; Do a fold based on the first value.
; Placeholder function for now.
(=> fold_not_empty {fn l} (fold fn (@ 0 l) (slice _ 1 (+ (# l) 1))))

; FUNCTIONS
(=> factorial (starcall * (range 1 (+ _ 1))))
(=> sgn (? _ (? (> _ 0) 1 -1) 0))
(=> ! (? _ 0 1))
(=> #/ (/ (starcall + _) (# _)))
(=> max_2 {a b} (? (> a b) a b))
(=> min_2 {a b} (? (< a b) a b))
(=> max (fold_not_empty max_2 _))
(=> min (fold_not_empty min_2 _))
(=> abs (* _ (sgn _)))
(=> ** {base exp} (starcall * (repeat exp base)))
(=> >> {i offset} (/ i (** 2 offset)))
(=> << {i offset} (* i (** 2 offset)))

(= avg #/)
(= pow **)
(= shfr >>)
(= shfl <<)
; EXAMPLES

(putl (<< 1 8)) ; -> 256
(putl (starcall * [10 20 30])) ; -> 6000
(putl (** 2 5)) ; -> 32
(putl (factorial 10)) ; -> 3628800
(putl (sgn -10)) ; -> -1
(putl (abs -289348)) ; -> 289348
